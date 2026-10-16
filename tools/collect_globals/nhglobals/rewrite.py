"""Rewrite pass: move every migrated variable into `struct <struct>` and
redirect all accesses through the thread-local pointer.

What happens to each migrated variable V (see analyze.Var):

  definition          replaced by
                        - nothing (global, macro access: the accessor macro
                          is emitted in the generated header), or
                        - `#define V (<ptr>-><field>)` at the definition site
                          (file static, macro access), or
                        - nothing (precise-rewrite access)
                      plus, when V had an initializer, a `static const`
                      template of the original initializer text next to it.
  extern declarations deleted (headers and in-file `extern`s).
  references          for precise-rewrite access, each identifier is
                      replaced by `(<ptr>-><field>)` at its spelling
                      location (so macro bodies are rewritten once).
  initializer         copied into the per-env struct by the generated
                      per-file init function (`memcpy` from the template, or
                      a runtime copy when the initializer names other
                      migrated objects, e.g. `&uarm`).
  private types       record/enum/typedef definitions that live in a .c file
                      are hoisted (verbatim) into the generated header so the
                      struct can hold the object by value.
"""
from __future__ import annotations

import os
import re
import sys
from collections import defaultdict

from . import clangutil as cu
from .ctok import tokenize

CXX_KEYWORDS = {
    "new", "delete", "class", "this", "template", "typename", "operator",
    "namespace", "private", "public", "protected", "virtual", "friend",
    "throw", "try", "catch", "export", "using", "explicit", "mutable",
    "bool", "true", "false", "and", "or", "not", "xor", "compl", "bitand",
    "bitor", "and_eq", "or_eq", "xor_eq", "not_eq", "typeid", "wchar_t",
    "const_cast", "static_cast", "dynamic_cast", "reinterpret_cast",
    "nullptr", "constexpr", "decltype", "noexcept", "static_assert",
    "thread_local", "alignas", "alignof", "char16_t", "char32_t",
}

SYSTEM_HEADER_FOR = {
    "bits/fcntl.h": "<fcntl.h>",
    "bits/types/struct_FILE.h": "<stdio.h>",
    "bits/types/FILE.h": "<stdio.h>",
    "bits/types/time_t.h": "<time.h>",
    "bits/termios-struct.h": "<termios.h>",
    "bits/struct_stat.h": "<sys/stat.h>",
    "pwd.h": "<pwd.h>",
    "bits/ioctl-types.h": "<sys/ioctl.h>",
    "sys/ucontext.h": "<sys/ucontext.h>",
    "bits/types/struct_timeval.h": "<sys/time.h>",
    "bits/types/struct_timespec.h": "<time.h>",
    "regex.h": "<regex.h>",
    "bits/types/sigset_t.h": "<signal.h>",
    "bits/types/struct_sigstack.h": "<signal.h>",
    "bits/sigaction.h": "<signal.h>",
}


class Rewriter:
    def __init__(self, model, analyzer, pointer="nh_g", struct="nh_globals", dry_run=False):
        self.m = model
        self.an = analyzer
        self.ptr = pointer
        self.struct = struct
        self.dry = dry_run
        self.files = {}          # path -> SourceFile
        self.deleted = defaultdict(list)   # path -> [(start, end)] ranges replaced
        self.hoisted = []        # ordered TypeDef list
        self.hoisted_set = set()
        self.header_includes = []   # '"foo.h"' / '<bar.h>'
        self.forward_decls = []
        self.per_file_init = defaultdict(list)   # init function owner file -> [code lines]
        self.notes = []
        self.mig = [v for v in model.vars.values() if v.migrate]
        self.mig_by_usr = {v.usr: v for v in self.mig}
        self.promote = [v for v in model.vars.values() if v.const_promote]
        self.none_files = set(model.none_files)
        # objects that NH_GLOBALS_NONE translation units see as plain globals
        self.none_vars = set()
        for v in self.mig:
            if v.file in self.none_files or any(r.tu in self.none_files for r in v.refs):
                if v.scope != "global":
                    raise RuntimeError(f"{v.name} ({v.scope}, {self.rel(v.file)}) is used by a NH_GLOBALS_NONE program; keep it or make it a global")
                self.none_vars.add(v.usr)
        self.central_init = []   # memcpy lines in the generated source
        self.templates = []      # (v, template name) for extern declarations

    # ------------------------------------------------------------ helpers
    _DIR_RE = re.compile(rb"^[ \t]*#[ \t]*(if|ifdef|ifndef|elif|else|endif)\b", re.M)

    def balanced_range(self, sf, start, end):
        """Widen [start, end) so that it contains whole preprocessor
        conditional groups: a declaration like
            #ifdef X
            volatile
            #endif
            struct T name;
        starts inside a group, and replacing it must take the group along."""
        data = sf.data
        depth = 0
        for mm in self._DIR_RE.finditer(data, start, end):
            kw = mm.group(1)
            if kw in (b"if", b"ifdef", b"ifndef"):
                depth += 1
            elif kw == b"endif":
                if depth == 0:
                    # opener lies before start: find it
                    d = 0
                    pos = start
                    found = None
                    for pm in reversed(list(self._DIR_RE.finditer(data, 0, start))):
                        k = pm.group(1)
                        if k == b"endif":
                            d += 1
                        elif k in (b"if", b"ifdef", b"ifndef"):
                            if d == 0:
                                found = pm.start()
                                break
                            d -= 1
                    if found is None:
                        raise RuntimeError(f"{self.rel(sf.path)}: unbalanced #endif near {start}")
                    start = found
                else:
                    depth -= 1
        if depth > 0:
            # opener inside, closer after end
            d = depth
            for pm in self._DIR_RE.finditer(data, end):
                k = pm.group(1)
                if k in (b"if", b"ifdef", b"ifndef"):
                    d += 1
                elif k == b"endif":
                    d -= 1
                    if d == 0:
                        end = data.find(b"\n", pm.start()) + 1
                        break
        return start, end

    def sf(self, path):
        f = self.files.get(path)
        if f is None:
            f = cu.SourceFile(path)
            self.files[path] = f
        return f

    def rel(self, path):
        return os.path.relpath(path, self.m.src_root)

    def acc(self, v):
        return f"{self.ptr}->{v.field_name}"

    def nhg(self, v):
        """Spelling used at rewritten references (also valid in
        NH_GLOBALS_NONE builds for globals)."""
        if v.opaque:
            return f"(*({self.opaque_type(v)}) NH_G({v.field_name}))"
        return f"NH_G({v.field_name})"

    def gen_acc(self, v):
        """Accessor usable from generated C code, where the accessor macros
        are in effect: a macro-access global must be spelled by its macro
        (a bare `nh_g->name` would expand the field name)."""
        if v.scope == "global" and v.access == "macro":
            return v.name
        if v.opaque:
            return f"(*({self.opaque_type(v)}) {self.acc(v)})"
        return f"({self.acc(v)})"

    def opaque_type(self, v):
        """Type to cast the raw storage to: pointer to the object's type."""
        sp = v.type_spelling
        if v.base_is_anon:
            sp = sp.replace(v.base_spelling, f"struct nh_anon_{v.field_name}")
        if "[" in sp:
            i = sp.index("[")
            return f"{sp[:i].rstrip()} (*){sp[i:]}"
        if "(*" in sp:
            i = sp.index("(*") + 2
            return f"{sp[:i]}*{sp[i:]}"
        return f"{sp} *"

    def note(self, msg):
        self.notes.append(msg)

    def owner_c_file(self, v):
        """The .c file whose init function copies V's initializer."""
        if v.file.endswith(".h"):
            return self.m.header_tu[v.file]
        return v.file

    # ------------------------------------------------------------ main
    def run(self):
        self.check_pointer_name()
        self.check_files_without_hack_h()
        self.plan_types()
        self.rewrite_definitions()
        self.rewrite_externs()
        self.rewrite_references()
        self.promote_consts()
        self.emit_init_functions()
        self.generate_header()
        self.generate_source()
        self.patch_hack_h()
        self.patch_config_h()
        if not self.dry:
            for f in self.files.values():
                f.write()
        return self.notes

    def includes_hack_h(self, path):
        hack = os.path.join(self.m.src_root, "include", "hack.h")
        tu = self.m.header_tu.get(path, path)
        return hack in self.m.tu_files.get(tu, set())

    def check_files_without_hack_h(self):
        """A file that never includes hack.h cannot reach the context pointer:
        it may only define migrated *globals* (their templates need no
        pointer; the accessors live in the generated header)."""
        for v in self.mig:
            if not self.includes_hack_h(v.file) and (v.scope != "global" or v.init_has_refs):
                raise RuntimeError(f"{self.rel(v.file)} does not include hack.h but defines per-env {v.scope} {v.name}; "
                                   f"add it to the keep list or include hack.h")

    def check_pointer_name(self):
        if self.ptr in self.m.names_declared or self.struct in self.m.names_declared:
            raise RuntimeError(f"pointer/struct name {self.ptr}/{self.struct} already used in the tree")

    # ------------------------------------------------------------ types
    def plan_types(self):
        """Hoist every private type a by-value field needs, dependencies
        first; forward-declare tags that are only pointed to; collect the
        headers the generated header has to include."""
        m = self.m
        hack_h = os.path.join(m.src_root, "include", "hack.h")
        reach = set()
        for tu, incs in m.tu_files.items():
            if hack_h in incs:
                pass
        # headers reachable from hack.h: use a TU that includes hack.h first
        # (any src/*.c) — the include closure of that TU minus the .c's own
        # later includes is a superset; good enough since we only add
        # includes that are missing everywhere.
        for tu, incs in m.tu_files.items():
            if os.path.basename(tu) == "decl.c":
                reach = set(incs)
                break

        def need_type(v):
            _, chain = self._chain(v)
            return chain  # list

        self.reach = reach
        for v in self.mig:
            if v.base_decl_file is None:
                continue
            chain = self._chain(v)
            byval = "ptr" not in chain
            bf = v.base_decl_file
            if bf.endswith(".c") or (bf.endswith(".h") and m.header_tu.get(bf) is not None):
                td = m.types.get(v.base_decl_usr)
                if td is None:
                    raise RuntimeError(f"no type record for {v.name} ({v.base_spelling})")
                if self._type_needs_opaque(td):
                    if not byval:
                        # a pointer only needs a forward declaration
                        if td.anon:
                            raise RuntimeError(f"{v.name}: pointer to anonymous private type")
                        fd = f"{td.kind} {td.name};"
                        if fd not in self.forward_decls:
                            self.forward_decls.append(fd)
                        continue
                    if v.scope == "global" and td.anon:
                        raise RuntimeError(f"global {v.name}: anonymous type cannot be hoisted ({td.deps_refs})")
                    v.opaque = True
                    self.note(f"{self.rel(v.file)}: {v.name} stored opaquely (type names {td.deps_refs})")
                    continue
                self.hoist(td)
            elif bf.startswith(m.src_root):
                is_tag = v.base_spelling.startswith(("struct ", "union ", "enum "))
                if bf not in reach and (byval or not is_tag):
                    if self.header_is_safe(bf):
                        inc = f'"{os.path.basename(bf)}"'
                        if inc not in self.header_includes:
                            self.header_includes.append(inc)
                    else:
                        v.opaque = True
                        self.note(f"{self.rel(v.file)}: {v.name} stored opaquely (type from {self.rel(bf)})")
                elif bf not in reach and not byval and is_tag:
                    fd = v.base_spelling.split("(")[0].strip() + ";"
                    if fd not in self.forward_decls:
                        self.forward_decls.append(fd)
            else:
                # system header
                if not byval and v.base_spelling.startswith(("struct ", "union ")):
                    fd = v.base_spelling + ";"
                    if fd not in self.forward_decls:
                        self.forward_decls.append(fd)
                    continue
                key = None
                for k in SYSTEM_HEADER_FOR:
                    if bf.endswith("/" + k):
                        key = k
                        break
                if key is None:
                    raise RuntimeError(f"field {v.name}: by-value system type {v.base_spelling} from {bf}: add to SYSTEM_HEADER_FOR")
                inc = SYSTEM_HEADER_FOR[key]
                if inc not in self.header_includes:
                    self.header_includes.append(inc)

    def _chain(self, v):
        return v.chain

    _safe_cache = {}

    def _suffix_macros_visible(self, v):
        """Are the macros spelling v's array size visible from the generated
        header (defined in hack.h's include closure or a header it adds)?"""
        for f in v.suffix_macro_files:
            if f in self.reach:
                continue
            if f'"{os.path.basename(f)}"' in self.header_includes:
                continue
            return False
        return True

    def header_is_safe(self, h):
        """May header h be included from hack.h (i.e. by every TU)? Not if a
        type/macro/enum/function name it declares is also used as an
        ordinary identifier (parameter, local, field, label) elsewhere --
        e.g. sp_lev.h's `typedef ... object` would break `f(object)`."""
        if h in self._safe_cache:
            return self._safe_cache[h]
        m = self.m
        clash = None
        for name, decls in m.names_declared.items():
            mine = [d for d in decls if d[1] == h and d[0] in ("typedef", "tag", "enumconst", "function", "macro")]
            if not mine:
                continue
            kinds = {d[0] for d in mine}
            if kinds == {"tag"}:
                # struct/union/enum tags have their own namespace: only
                # another, different definition of the same tag clashes
                odef = [d for d in decls if d[1] != h and d[0] == "tag" and d[2] not in {x[2] for x in mine}]
                if odef:
                    clash = (name, odef[0])
                    break
                continue
            others = [d for d in decls if d[1] != h and d[0] in ("param", "var", "field", "label")]
            if others:
                clash = (name, others[0])
                break
            # the same type name defined differently elsewhere
            odef = [d for d in decls if d[1] != h and d[0] in ("typedef", "tag") and d[2] not in {x[2] for x in mine}]
            if odef:
                clash = (name, odef[0])
                break
        safe = clash is None
        if not safe:
            self.note(f"header {self.rel(h)} not includable globally: {clash[0]} also declared as {clash[1][0]} in {self.rel(clash[1][1])}")
        self._safe_cache[h] = safe
        return safe

    def _type_needs_opaque(self, td):
        if td.deps_refs:
            return True
        for h in td.hdr_deps:
            if h not in self.reach and h.startswith(self.m.src_root) and not self.header_is_safe(h):
                return True
        return any(self._type_needs_opaque(self.m.types[d]) for d in td.deps_types if d in self.m.types)

    def hoist(self, td):
        if td.usr in self.hoisted_set:
            return
        self.hoisted_set.add(td.usr)
        for dep in td.deps_types:
            d = self.m.types.get(dep)
            if d is not None:
                self.hoist(d)
        for h in td.hdr_deps:
            if h not in self.reach and h.startswith(self.m.src_root):
                inc = f'"{os.path.basename(h)}"'
                if inc not in self.header_includes:
                    self.header_includes.append(inc)
        self.hoisted.append(td)
        # remove the definition from the .c (the header now provides it) --
        # unless it is spelled inside a migrated variable's own declaration
        # statement, which gets replaced wholesale anyway.
        inside_stmt = any(v.file == td.file and v.stmt_start <= td.start < v.stmt_end for v in self.mig)
        if (td.kind == "typedef" or td.name) and not inside_stmt:
            sf = self.sf(td.file)
            end = td.end
            # include trailing ';'
            toks = tokenize(sf.data, end, min(len(sf.data), end + 50))
            if toks and toks[0].text == ";":
                end = toks[0].end
            s = td.start
            sf.replace(s, end, f"/* {td.kind} {td.name} moved to {self.struct}.h */")
            self.deleted[td.file].append((s, end))

    # ------------------------------------------------------------ declarations
    def rewrite_definitions(self):
        """Replace each definition statement once. Single-declarator
        statements with a (constant) initializer keep their initializer in
        place and only get their head rewritten into the template."""
        stmts = defaultdict(list)
        for v in self.mig:
            stmts[(v.file, v.stmt_start)].append(v)
        for (path, start), vs in stmts.items():
            vs.sort(key=lambda v: v.name_offset)
            v0 = vs[0]
            sf = self.sf(path)
            end = v0.stmt_end
            names = v0.stmt_names
            if len(names) == 1 and v0.init is not None and not v0.init_has_refs \
                    and not _is_zero_init(v0.init) and v0.scope != "local":
                # head rewrite: `[static] T name[N] =` -> `const T nh_tmpl[N] =`
                hstart, hend = self.balanced_range(sf, start, v0.init_start)
                if hend != v0.init_start:
                    raise RuntimeError(f"{self.rel(path)}: {v0.name}: conditional group spans the initializer")
                start = hstart
                tmpl = f"nh_tmpl_{v0.field_name}"
                lines = self._access_lines(v0)
                lines.append(f"{self.decl_text(v0, tmpl, const=True, keep_unsized=True)} =")
                new = "\n".join(lines) + "\n"
                if path in self.none_files:
                    orig = sf.text(start, v0.init_start).rstrip()
                    new = f"#ifdef NH_GLOBALS_NONE\n{orig}\n#else\n{new}#endif\n"
                sf.replace(start, v0.init_start, new)
                self.deleted[path].append((start, v0.init_start))
                self._register_template(v0, tmpl)
                continue
            start, end = self.balanced_range(sf, start, end)
            migrated = {v.name for v in vs}
            others = [n for n in names if n not in migrated]
            out = self.inline_type_text(vs, path, start, end)
            if others:
                # keep the non-migrated declarators of this statement
                spec, decls = split_statement(sf.data, start, end)
                kept = [d for d in decls if declarator_name(d) in others]
                out.append(spec + " " + ", ".join(kept) + ";")
                self.note(f"{self.rel(path)}: statement split, kept {others}")
            before_func = []
            for v in vs:
                lines, tmpl_lines = self.definition_replacement(v)
                out.extend(lines)
                if v.scope == "local":
                    before_func.extend(tmpl_lines)
                else:
                    out.extend(tmpl_lines)
            text = "\n".join(out)
            if path in self.none_files:
                text = f"#ifdef NH_GLOBALS_NONE\n{sf.text(start, end).rstrip()}\n#else\n{text}\n#endif"
            sf.replace(start, end, text)
            self.deleted[path].append((start, end))
            if before_func:
                # function-static templates must be visible to the init
                # function: put them right before the enclosing function
                at = v0.func_start
                sf.insert(at, "\n".join(before_func) + "\n\n")

    def inline_type_text(self, vs, path, stmt_start, stmt_end):
        """If one of these variables' base type is *defined* inside this very
        statement (`E struct sinfo { ... } program_state;`) and is not being
        hoisted, the definition has to survive the statement's removal."""
        out = []
        for v in vs:
            if v.base_decl_file == path and v.base_decl_start is not None \
                    and stmt_start <= v.base_decl_start < stmt_end \
                    and v.base_decl_usr not in self.hoisted_set:
                sf = self.sf(path)
                text = sf.text(v.base_decl_start, v.base_decl_end).rstrip()
                if v.base_is_anon:
                    td = self.m.types.get(v.base_decl_usr)
                    text = f"{td.kind} nh_anon_{v.field_name} " + text[text.index("{"):]
                if text not in out:
                    out.append(text)
        return [t + ";" for t in out]

    def _access_lines(self, v):
        if v.scope == "static" and v.access == "macro":
            if v.opaque:
                return [f"#define {v.name} (*({self.opaque_type(v)}) {self.acc(v)})"]
            return [f"#define {v.name} ({self.acc(v)})"]
        if v.scope == "global" and v.access == "macro":
            return [f"/* {v.name}: per-env, see {self.struct}.h */"]
        return [f"/* {v.name}: per-env {self.acc(v)} */"]

    def definition_replacement(self, v):
        """(lines replacing the declaration, template lines)"""
        lines = self._access_lines(v)
        tmpl_lines = []
        if v.init is not None and not _is_zero_init(v.init):
            init = self.init_text(v)
            if v.init_has_refs:
                # runtime copy in the init function (initializer names other
                # per-env objects, whose addresses are not constants)
                self.per_file_init[self.owner_c_file(v)].append(
                    "    {\n"
                    f"        {self.decl_text(v, 'nh_tmp', const=False)} = {init};\n"
                    f"        memcpy(&{self.gen_acc(v)}, &nh_tmp, sizeof nh_tmp);\n"
                    "    }")
            else:
                tmpl = f"nh_tmpl_{v.field_name}"
                tmpl_lines.append(f"{self.decl_text(v, tmpl, const=True, keep_unsized=True)} = {init};")
                self._register_template(v, tmpl)
        return lines, tmpl_lines

    def _register_template(self, v, tmpl):
        if v.opaque:
            self.per_file_init[self.owner_c_file(v)].append(
                f"    memcpy({self.acc(v)}, &{tmpl}, sizeof {tmpl});")
        else:
            self.templates.append((v, tmpl))
            self.central_init.append(f"    if (&{tmpl})\n        memcpy(&{self.gen_acc(v)}, &{tmpl}, sizeof {self.gen_acc(v)});")

    def init_text(self, v):
        """Initializer text with the precise-rewrite references inside it
        already redirected."""
        sf = self.sf(v.file)
        text = sf.data[v.init_start:v.init_end]
        edits = []
        for other in self.mig:
            if other.access != "rewrite":
                continue
            for r in other.refs:
                if r.file == v.file and v.init_start <= r.offset < v.init_end and not r.in_macro:
                    edits.append((r.offset - v.init_start, len(other.name), self.nhg(other)))
        out = bytearray()
        pos = 0
        for off, ln, rep in sorted(edits):
            out += text[pos:off]
            out += rep.encode()
            pos = off + ln
        out += text[pos:]
        return out.decode("utf-8", "surrogateescape").strip()

    def decl_text(self, v, name, const=False, keep_unsized=False, in_header=False):
        """`<base> <prefix>NAME<suffix>` with the object const-qualified
        when asked (const binds to the pointer for pointer objects).
        in_header: the text must compile from the generated header, so array
        sizes that use .c-private macros are spelled numerically."""
        base = v.base_spelling
        if v.base_is_anon:
            base = f"struct nh_anon_{v.field_name}"
        if v.forced and not const:
            # the object used to be const; the per-env copy is written to
            base = re.sub(r"^const\s+", "", base)
            base = re.sub(r"\s+const$", "", base)
        if v.decl_has_macro and not v.suffix.startswith("["):
            return self._decl_from_spelling(v, name, const)
        prefix = v.prefix
        suffix = v.suffix
        if v.forced and not const and "*" in prefix:
            prefix = prefix.replace("const", "").replace("  ", " ")
        if suffix.startswith("[]") and not keep_unsized:
            n = v.elem_count
            if not n:
                raise RuntimeError(f"{v.name}: cannot size [] array")
            suffix = f"[{n}]" + suffix[2:]
        if in_header and (v.suffix_private or not self._suffix_macros_visible(v)) and v.dims:
            it = iter(v.dims)
            suffix = re.sub(r"\[[^\]]*\]", lambda mm: f"[{next(it)}]", suffix)
        if const:
            if "*" in prefix:
                i = prefix.rfind("*")
                prefix = prefix[: i + 1] + "const " + prefix[i + 1:]
            else:
                base = "const " + base if not base.startswith("const ") else base
        return f"{base} {prefix}{name}{suffix}"

    def _decl_from_spelling(self, v, name, const):
        """Build the declarator from libclang's type spelling (used when the
        source declarator was produced by a macro such as NDECL/FDECL)."""
        sp = v.type_spelling
        if v.base_is_anon:
            sp = sp.replace(v.base_spelling, f"struct nh_anon_{v.field_name}")
        if (v.suffix_private or not self._suffix_macros_visible(v)) and v.dims:
            it = iter(v.dims)
            sp = re.sub(r"\[[^\]]*\]", lambda mm: f"[{next(it)}]", sp)
        nm = ("const " if const and "(*" not in sp and "*" not in sp else "") + name
        if "(*" in sp:
            i = sp.index("(*") + 2
            return sp[:i] + ("const " if const else "") + name + sp[i:]
        if "[" in sp:
            i = sp.index("[")
            head = sp[:i].rstrip()
            if const:
                if head.endswith("*"):
                    head += "const"
                else:
                    head = "const " + head
            return f"{head} {name}{sp[i:]}"
        if const and sp.rstrip().endswith("*"):
            return f"{sp}const {name}"
        return f"{sp} {nm}"

    def field_decl(self, v):
        if v.opaque:
            return f"unsigned char {v.field_name}[{v.size}] __attribute__((aligned({v.align})))"
        return self.decl_text(v, v.field_name, const=False, in_header=True)

    # ------------------------------------------------------------ externs
    def rewrite_externs(self):
        stmts = {}
        for v in self.mig:
            for e in v.externs:
                stmts.setdefault((e.file, e.stmt_start), (e, set()))[1].add(v.name)
        for (path, start), (e, migrated) in stmts.items():
            sf = self.sf(path)
            start, stmt_end = self.balanced_range(sf, start, e.stmt_end)
            e.stmt_end = stmt_end
            others = [n for n in e.names if n not in migrated]
            vs = [v for v in self.mig if v.name in migrated and any(x.file == path and x.stmt_start == start for x in v.externs)]
            keep_types = self.inline_type_text(vs, path, start, e.stmt_end)
            if others:
                spec, decls = split_statement(sf.data, start, e.stmt_end)
                kept = [d for d in decls if declarator_name(d) in others]
                sf.replace(start, e.stmt_end, spec + " " + ", ".join(kept) + ";")
                self.note(f"{self.rel(path)}: extern statement split, kept {others}")
            elif any(v.usr in self.none_vars for v in vs):
                # plain-global builds (makedefs & co) still need the extern
                sf.replace(start, e.stmt_end,
                           "\n".join(keep_types + [f"#ifdef NH_GLOBALS_NONE\n{sf.text(start, e.stmt_end).rstrip()}\n#endif"]))
            else:
                sf.replace(start, e.stmt_end, "\n".join(keep_types + [f"/* {', '.join(e.names)}: per-env, see {self.struct}.h */"]))
            self.deleted[path].append((start, e.stmt_end))

    # ------------------------------------------------------------ references
    def rewrite_references(self):
        seen = {}
        for v in self.mig:
            if v.access != "rewrite":
                continue
            for r in v.refs:
                key = (r.file, r.offset)
                if key in seen:
                    if seen[key] != v.usr:
                        raise RuntimeError(f"spelling location {self.rel(r.file)}:{r.offset} refers to two objects")
                    continue
                seen[key] = v.usr
                if self._inside_deleted(r.file, r.offset):
                    continue
                if r.file in self.an.exclude:
                    self.note(f"excluded file {self.rel(r.file)} references {v.name} (rewrite by hand)")
                    continue
                if r.in_macro:
                    for (mf, moff) in self.macro_sites(v, r):
                        mkey = (mf, moff)
                        if mkey in self.macro_done:
                            continue
                        self.macro_done.add(mkey)
                        if mf in self.an.exclude:
                            self.note(f"excluded file {self.rel(mf)} macro references {v.name} (rewrite by hand)")
                            continue
                        self.sf(mf).replace(moff, moff + len(v.name), self.nhg(v))
                    continue
                sf = self.sf(r.file)
                ident = sf.data[r.offset:r.offset + len(v.name)].decode()
                if ident != v.name:
                    raise RuntimeError(f"{self.rel(r.file)}:{r.offset}: expected {v.name}, found {ident!r}")
                sf.replace(r.offset, r.offset + len(v.name), self.nhg(v))

    macro_done = set()

    def macro_sites(self, v, r):
        """Token positions to rewrite for a reference that libclang placed at
        a macro use site: the name spelled in the macro arguments at the use
        site and/or inside the (possibly nested) macro bodies."""
        m = self.m
        inst = m.inst_at.get((r.file, r.offset))
        if inst is None:
            raise RuntimeError(f"{self.rel(r.file)}:{r.offset}: {v.name} reference not at a macro instantiation")
        mname, mend = inst
        sites = []
        data = self.sf(r.file).data
        # (1) arguments at the use site
        toks = tokenize(data, r.offset, mend)
        visited = set()
        for k, t in enumerate(toks[1:], 1):
            prev = toks[k - 1].text
            if t.kind != "ident" or prev in (".", "->"):
                continue
            if t.text == v.name:
                sites.append((r.file, t.start))
            elif t.text in self.m.macro_defs:
                # the argument itself uses a macro (e.g. levl[x][y])
                self._body_sites(v, t.text, r.tu, sites, visited)
        # (2) macro bodies, recursively
        self._body_sites(v, mname, r.tu, sites, visited)
        if not sites:
            raise RuntimeError(f"{self.rel(r.file)}:{r.offset}: cannot locate {v.name} in expansion of {mname}")
        return sites

    def _macro_defs_for(self, name, tu):
        """Every definition of macro `name` visible from TU (a file may
        #define/#undef the same name several times: all bodies count)."""
        defs = self.m.macro_defs.get(name, [])
        vis = self.m.tu_files.get(tu, set()) | {tu}
        return [d for d in defs if d[0] in vis]

    def _body_sites(self, v, mname, tu, sites, visited):
        if mname in visited:
            return
        visited.add(mname)
        for d in self._macro_defs_for(mname, tu):
            self._body_sites_def(v, mname, d, tu, sites, visited)

    def _body_sites_def(self, v, mname, d, tu, sites, visited):
        mfile, ms, me, text = d
        data = self.sf(mfile).data
        toks = tokenize(data, ms, me)
        if not toks:
            return
        params = set()
        i = 1
        if len(toks) > 1 and toks[1].text == "(" and toks[1].start == toks[0].end:
            j = 1
            while toks[j].text != ")":
                if toks[j].kind == "ident":
                    params.add(toks[j].text)
                j += 1
            i = j + 1
        body = toks[i:]
        hits = []
        nested = []
        for k, t in enumerate(body):
            if t.kind != "ident":
                continue
            prev = body[k - 1].text if k > 0 else ""
            if t.text == v.name and t.text not in params and prev not in (".", "->"):
                hits.append((mfile, t.start))
            elif t.text in self.m.macro_defs and t.text != mname:
                nested.append(t.text)
        if hits:
            # consistency: every instantiation of this macro must resolve
            # v.name to v (never to a local/param or another object)
            for f, uses in self.m.macro_uses.items():
                for name, s, e in uses:
                    if name != mname:
                        continue
                    usrs = self.m.name_refs_at.get((f, s), {}).get(v.name)
                    if usrs and usrs != {v.usr}:
                        raise RuntimeError(f"macro {mname} body names {v.name} but an instantiation at {self.rel(f)}:{s} resolves it to {usrs}")
            sites.extend(hits)
        for n in nested:
            self._body_sites(v, n, tu, sites, visited)

    def _inside_deleted(self, path, off):
        for s, e in self.deleted.get(path, []):
            if s <= off < e:
                return True
        return False

    # ------------------------------------------------------------ const promotion
    def promote_consts(self):
        for v in self.promote:
            if len(v.stmt_names) != 1:
                raise RuntimeError(f"--const-promote {v.name}: multi-declarator statement not supported")
            sf = self.sf(v.file)
            if v.init is None:
                raise RuntimeError(f"--const-promote {v.name}: no initializer")
            head = f"{self.decl_text(v, v.name, const=True, keep_unsized=True)} ="
            sf.replace(v.stmt_start, v.init_start, head + "\n")
            for e in v.externs:
                if len(e.names) != 1:
                    raise RuntimeError(f"--const-promote {v.name}: multi-declarator extern")
                esf = self.sf(e.file)
                esf.replace(e.stmt_start, e.stmt_end, f"extern {self.decl_text(v, v.name, const=True, keep_unsized=True)};")
            self.note(f"const-promoted {v.name} ({self.rel(v.file)}, {len(v.externs)} extern decls)")

    # ------------------------------------------------------------ init fns
    def init_fn_name(self, path):
        return "nh_init_" + _file_key(self.rel(path))

    def emit_init_functions(self):
        for path, lines in self.per_file_init.items():
            if not self.includes_hack_h(path):
                raise RuntimeError(f"{self.rel(path)} needs a runtime initializer copy but does not include hack.h")
            sf = self.sf(path)
            fn = self.init_fn_name(path)
            body = "\n".join(lines)
            guard = f"NH_INIT_{_file_key(self.rel(path)).upper()}_DONE"
            code = (f"\n\n/* {self.struct}: copy this file's initialized per-env objects into the\n"
                    f" * current context. Generated by tools/collect_globals. */\n"
                    f"#ifndef {guard}\n#define {guard}\n"
                    f"void\n{fn}(void)\n{{\n{body}\n}}\n#endif\n")
            sf.insert(len(sf.data), code)

    # ------------------------------------------------------------ header
    def generate_header(self):
        m = self.m
        S = self.struct
        P = self.ptr
        out = []
        out.append(f"/* {S}.h -- every mutable file-scope / function-static object of the\n"
                   f" * game, collected into one per-environment struct.\n"
                   f" *\n"
                   f" * GENERATED by tools/collect_globals -- do not edit by hand. Re-run the\n"
                   f" * tool on a pristine tree instead.\n"
                   f" *\n"
                   f" * Access goes through the thread-local pointer `{P}`: object-like macros\n"
                   f" * (below) keep the original identifiers working for most objects; the\n"
                   f" * ones whose names collide with struct fields or other identifiers were\n"
                   f" * rewritten at every reference to `{P}->name`. */")
        out.append(f"#ifndef {S.upper()}_H\n#define {S.upper()}_H\n")
        for inc in self.header_includes:
            out.append(f"#include {inc}")
        if self.header_includes:
            out.append("")
        if self.forward_decls:
            out.append("/* opaque types only pointed to from the struct */")
            for fd in self.forward_decls:
                out.append(fd)
            out.append("")
        if self.hoisted:
            out.append("/* --- types hoisted from their defining .c files -------------------- */")
            for td in self.hoisted:
                out.append(f"/* from {self.rel(td.file)} */")
                for name, f, s, e, text in _dedupe_macros(td.deps_macros):
                    out.append(f"#ifndef {name}\n#define {text}\n#endif")
                text = self.hoisted_text(td)
                # members named after C++ keywords (`new`): the C++ translation
                # units that include hack.h never touch them, so rename them
                # there only (same layout).
                kws = sorted({t.text for t in tokenize(text.encode(), 0, len(text.encode()))
                              if t.kind == "ident" and t.text in CXX_KEYWORDS})
                if kws:
                    out.append("#ifdef __cplusplus")
                    for k in kws:
                        out.append(f"#define {k} nh_cxx_{k}_")
                    out.append("#endif")
                out.append(text)
                if kws:
                    out.append("#ifdef __cplusplus")
                    for k in kws:
                        out.append(f"#undef {k}")
                    out.append("#endif")
                out.append("")
        out.append(f"struct {S} {{")
        cur = None
        for v in sorted(self.mig, key=lambda v: (self.rel(v.file), v.name_offset)):
            if v.file != cur:
                cur = v.file
                out.append(f"    /* ---- {self.rel(v.file)} ---- */")
            origin = v.name if v.scope == "global" else (f"static {v.name}" if v.scope == "static" else f"{v.func}(): static {v.name}")
            out.append(f"    {self.field_decl(v)}; /* {origin} */")
        out.append("};\n")
        out.append("#ifndef NH_THREAD_LOCAL\n#define NH_THREAD_LOCAL __thread\n#endif")
        out.append(f"extern NH_THREAD_LOCAL struct {S} *{P};\n")
        out.append(f"/* Zero a context and re-apply every static initializer (the .data image\n"
                   f" * of the original globals). Sets `{P}` to it. */")
        out.append(f"void {S}_init(struct {S} *);")
        out.append(f"size_t {S}_size(void);")
        out.append("/* per-file initializer copies (generated at the end of each file; weak so\n"
                   " * that programs linking a subset of the game can skip them) */")
        for path in sorted(self.per_file_init):
            out.append(f"void {self.init_fn_name(path)}(void) __attribute__((weak));")
        out.append("")
        out.append("/* --- accessor macros for the objects that kept their names ------------ */")
        for v in sorted(self.mig, key=lambda v: v.name):
            if v.scope == "global" and v.access == "macro":
                if v.opaque:
                    out.append(f"#define {v.name} (*({self.opaque_type(v)}) {self.acc(v)})")
                else:
                    out.append(f"#define {v.name} ({self.acc(v)})")
        out.append(f"\n#endif /* {S.upper()}_H */\n")
        path = os.path.join(m.src_root, "include", f"{S}.h")
        self.generated_header = "\n".join(out)
        if not self.dry:
            with open(path, "w") as f:
                f.write(self.generated_header)

    def hoisted_text(self, td):
        text = td.text
        if td.kind in ("struct", "union", "enum") and (td.anon or not td.name):
            # anonymous: give it a tag named after the (single) field using it
            users = [v for v in self.mig if v.base_decl_usr == td.usr]
            tag = f"nh_anon_{users[0].field_name}"
            body = text[text.index("{"):]
            return f"{td.kind} {tag} {body};"
        if td.kind in ("struct", "union", "enum"):
            return text + ";"
        return text + ";" if not text.rstrip().endswith(";") else text

    # ------------------------------------------------------------ source
    def generate_source(self):
        m = self.m
        S = self.struct
        P = self.ptr
        out = []
        out.append(f"/* {S}.c -- per-environment context storage and initialisation.\n"
                   f" * GENERATED by tools/collect_globals -- do not edit by hand. */\n")
        out.append('#include "hack.h"\n')
        out.append(f"NH_THREAD_LOCAL struct {S} *{P};\n")
        out.append("/* The .data image of every initialized object: `const` templates left at\n"
                   " * the original definition sites. Weak, so a program that links only a\n"
                   " * few game files (lev_comp) simply skips the templates it lacks. */")
        for v, tmpl in sorted(self.templates, key=lambda t: t[1]):
            out.append(f"extern {self.decl_text(v, tmpl, const=True, in_header=True)} __attribute__((weak));")
        out.append("")
        out.append(f"size_t\n{S}_size(void)\n{{\n    return sizeof(struct {S});\n}}\n")
        out.append(f"void\n{S}_init(struct {S} *g)\n{{\n    memset((genericptr_t) g, 0, sizeof *g);\n    {P} = g;")
        out.extend(self.central_init)
        for path in sorted(self.per_file_init):
            out.append(f"    if ({self.init_fn_name(path)})\n        {self.init_fn_name(path)}();")
        out.append("}\n")
        out.append(f"#ifdef NH_GLOBALS_STANDALONE\n"
                   f"/* Single-context programs (the build-time utilities): give them one\n"
                   f" * context before main() runs. */\n"
                   f"static struct {S} nh_standalone_ctx;\n"
                   f"__attribute__((constructor)) static void\n{S}_standalone(void)\n{{\n"
                   f"    {S}_init(&nh_standalone_ctx);\n}}\n#endif\n")
        path = os.path.join(m.src_root, "src", f"{S}.c")
        self.generated_source = "\n".join(out)
        if not self.dry:
            with open(path, "w") as f:
                f.write(self.generated_source)

    def patch_config_h(self):
        path = os.path.join(self.m.src_root, "include", "config.h")
        sf = self.sf(path)
        i = sf.data.rfind(b"#endif")
        sf.insert(i, (
            "/* Per-environment globals (tools/collect_globals): every mutable global of\n"
            f" * the game lives in struct {self.struct}, reached through the thread-local\n"
            f" * pointer `{self.ptr}` (see {self.struct}.h). NH_G(x) spells such an access\n"
            " * where the plain identifier cannot be an accessor macro. Programs built\n"
            " * with NH_GLOBALS_NONE (makedefs & co, which run before the generated\n"
            " * headers exist) keep plain globals instead. */\n"
            "#ifdef NH_GLOBALS_NONE\n#define NH_G(x) x\n#else\n"
            f"#define NH_G(x) ({self.ptr}->x)\n#endif\n\n"))

    def patch_hack_h(self):
        path = os.path.join(self.m.src_root, "include", "hack.h")
        sf = self.sf(path)
        i = sf.data.rfind(b"#endif")
        sf.insert(i, f'#include "{self.struct}.h" /* per-env globals (generated) */\n\n')


# ---------------------------------------------------------------- utilities
def split_statement(data, start, end):
    """Split a declaration statement into (specifier text, [declarator texts])."""
    toks = tokenize(data, start, end)
    # find declarator start: first '*' or '(' at depth 0 before the first
    # depth-0 '=', ',', ';', '['; else the identifier just before it.
    depth = 0
    first_stop = None
    for i, t in enumerate(toks):
        if depth == 0 and t.text in ("=", ",", ";", "["):
            first_stop = i
            break
        if t.text in ("(", "[", "{"):
            depth += 1
        elif t.text in (")", "]", "}"):
            depth -= 1
    if first_stop is None:
        raise ValueError("cannot split statement")
    dstart = None
    for i in range(first_stop):
        if toks[i].text in ("*", "(") and _depth_at(toks, i) == 0:
            dstart = i
            break
    if dstart is None:
        dstart = first_stop - 1
    spec = data[toks[0].start:toks[dstart].start].decode("utf-8", "surrogateescape").strip()
    # declarators separated by depth-0 commas
    decls = []
    cur_start = toks[dstart].start
    depth = 0
    for t in toks[dstart:]:
        if t.text in ("(", "[", "{"):
            depth += 1
        elif t.text in (")", "]", "}"):
            depth -= 1
        elif depth == 0 and t.text in (",", ";"):
            decls.append(data[cur_start:t.start].decode("utf-8", "surrogateescape").strip())
            cur_start = t.end
    return spec, decls


def _depth_at(toks, idx):
    d = 0
    for t in toks[:idx]:
        if t.text in ("(", "[", "{"):
            d += 1
        elif t.text in (")", "]", "}"):
            d -= 1
    return d


def declarator_name(decl_text):
    """The identifier declared by a declarator fragment like `*foo[3] = {..}`."""
    m = re.match(r"^[\s(*]*(?:const\s+)?[\s(*]*([A-Za-z_][A-Za-z0-9_]*)", decl_text)
    if not m:
        raise ValueError(f"no name in declarator {decl_text!r}")
    return m.group(1)


_ZERO_RE = re.compile(r"^(\(\s*[\w\s\*]+\)\s*)?(0|0L|0l|NULL|FALSE|'\\0'|\{\s*0\s*\}|DUMMY)$")


def _is_zero_init(init):
    return bool(_ZERO_RE.match(init.strip()))


def _file_key(rel):
    return os.path.basename(rel).replace(".", "_").replace("-", "_")


def _dedupe_macros(deps):
    seen = set()
    out = []
    for d in deps:
        if d[0] in seen:
            continue
        seen.add(d[0])
        out.append(d)
    return out


def apply_all(model, analyzer, pointer="nh_g", struct="nh_globals", dry_run=False):
    rw = Rewriter(model, analyzer, pointer=pointer, struct=struct, dry_run=dry_run)
    notes = rw.run()
    for n in notes:
        print("note:", n, file=sys.stderr)
    mig = rw.mig
    print(f"migrated {len(mig)} objects into struct {struct} "
          f"({sum(1 for v in mig if v.access == 'macro')} via macros, "
          f"{sum(1 for v in mig if v.access == 'rewrite')} via reference rewriting, "
          f"{len(rw.hoisted)} private types hoisted)", file=sys.stderr)
    return rw
