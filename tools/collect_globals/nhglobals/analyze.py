"""Analysis pass: find every static-storage variable in the library's
translation units, where it is referenced, and what would collide with an
accessor macro of the same name.

The output is a `Model` that the rewriter/generator consume. Nothing here
edits files.
"""
from __future__ import annotations

import os
import sys
from collections import defaultdict
from dataclasses import dataclass, field

import clang.cindex as ci

from . import clangutil as cu
from .ctok import KEYWORDS, find_matching, tokenize

CK = ci.CursorKind
SC = ci.StorageClass

# Cursor kinds whose *names* an object-like accessor macro would clobber.
NAMED_DECL_KINDS = {
    CK.FIELD_DECL, CK.PARM_DECL, CK.VAR_DECL, CK.FUNCTION_DECL,
    CK.ENUM_CONSTANT_DECL, CK.TYPEDEF_DECL, CK.STRUCT_DECL, CK.UNION_DECL,
    CK.ENUM_DECL, CK.MACRO_DEFINITION, CK.LABEL_STMT,
}


@dataclass
class Ref:
    file: str          # spelling file
    offset: int        # spelling byte offset of the identifier
    exp_file: str      # expansion file
    exp_offset: int
    in_macro: bool     # spelled inside a macro definition
    tu: str            # TU main file


@dataclass
class ExternDecl:
    file: str
    stmt_start: int
    stmt_end: int      # exclusive, past the ';'
    names: tuple       # all declarator names in that statement
    in_function: str | None


@dataclass
class Var:
    usr: str
    name: str
    scope: str                 # 'global' | 'static' | 'local'
    file: str                  # defining file
    func: str | None           # enclosing function for locals
    name_offset: int
    ext_start: int
    ext_end: int
    stmt_start: int
    stmt_end: int              # exclusive, past ';'
    stmt_names: tuple          # every declarator name of the statement
    base_spelling: str         # e.g. 'struct obj', 'const char'
    prefix: str                # declarator text before the name ('*', '(*')
    suffix: str                # declarator text after the name ('[N][M]', ')(int)')
    init: str | None           # initializer text (after '=')
    init_start: int | None
    init_end: int | None
    ctype: ci.Type = None
    chain: list = field(default_factory=list)   # ('ptr'|'arr'|'fn') steps down to the base type
    elem_count: int | None = None               # first array dimension (deduced)
    base_decl_usr: str | None = None
    base_decl_file: str | None = None
    base_is_anon: bool = False
    const_object: bool = False
    tentative: bool = False
    refs: list = field(default_factory=list)
    externs: list = field(default_factory=list)
    init_refs: set = field(default_factory=set)   # USRs of migrated vars named in init
    init_has_refs: bool = False
    field_name: str = ""
    access: str = "macro"      # 'macro' | 'rewrite'
    collisions: list = field(default_factory=list)
    keep: bool = False         # whitelisted: leave as a process global
    readonly: bool = False     # never written (per the reference build): leave alone
    const_promote: bool = False  # never written: make it `const` instead of per-env
    stmt_has_static_kw: bool = False
    func_start: int | None = None   # offset of the enclosing function definition
    dims: list = field(default_factory=list)      # array sizes from the AST, outermost first
    suffix_private: bool = False    # array size spelled with .c-private macros / other objects
    suffix_macro_files: list = field(default_factory=list)  # where the macros in the array size are defined
    decl_has_macro: bool = False    # declarator built by a macro (NDECL/FDECL): use type spelling
    type_spelling: str = ""         # libclang spelling of the full type
    base_decl_start: int | None = None
    base_decl_end: int | None = None
    size: int = 0                  # sizeof / alignof from the AST
    align: int = 0
    opaque: bool = False           # stored as raw bytes; the type stays private to its .c
    forced: bool = False           # const object whose initializer takes a per-env address

    @property
    def migrate(self):
        if self.forced:
            return True
        return not (self.const_object or self.keep or self.readonly or self.const_promote)

    @property
    def key(self):
        return self.usr


@dataclass
class TypeDef:
    """A record/enum/typedef declaration that has to be hoisted into the
    generated header because it lives in a .c file."""
    usr: str
    name: str            # tag or typedef name; '' if anonymous
    kind: str            # 'struct' | 'union' | 'enum' | 'typedef'
    file: str
    start: int
    end: int             # extent end (without trailing ';' for records)
    text: str
    deps_types: list = field(default_factory=list)   # usrs
    deps_macros: list = field(default_factory=list)  # (name, file, start, end, text)
    hdr_deps: list = field(default_factory=list)     # header files needed
    is_definition: bool = True
    anon: bool = False
    deps_refs: list = field(default_factory=list)    # objects named inside (sizeof x): not hoistable


def _dd_list():
    return defaultdict(list)


def _dd_set():
    return defaultdict(set)


def _dd_dd_set():
    return defaultdict(_dd_set)


@dataclass
class Model:
    src_root: str
    vars: dict = field(default_factory=dict)          # usr -> Var
    by_name: dict = field(default_factory=_dd_list)
    names_declared: dict = field(default_factory=_dd_set)  # name -> {(kind, file, usr)}
    macros: dict = field(default_factory=dict)        # name -> (file, start, end, text) first definition
    macro_defs: dict = field(default_factory=_dd_list)  # name -> [(file, start, end, text)]
    macro_uses: dict = field(default_factory=_dd_list)  # file -> [(name, start, end)]
    inst_at: dict = field(default_factory=dict)       # (file, start) -> (name, end)
    inst_def: dict = field(default_factory=dict)      # (file, start) -> file defining the macro used there
    name_refs_at: dict = field(default_factory=_dd_dd_set)  # (file, off) -> name -> {usr}
    types: dict = field(default_factory=dict)         # usr -> TypeDef (hoist candidates)
    tu_files: dict = field(default_factory=dict)      # tu main file -> set(files included)
    ref_only_tus: set = field(default_factory=set)
    none_files: set = field(default_factory=set)
    header_tu: dict = field(default_factory=dict)     # header with static defs -> including TU
    hack_h_includes: set = field(default_factory=set)
    diagnostics: list = field(default_factory=list)
    static_in_header: list = field(default_factory=list)


def _in_tree(model, path):
    return path is not None and os.path.abspath(path).startswith(model.src_root + os.sep)


class Analyzer:
    def __init__(self, src_root, compile_commands, exclude_files=(), verbose=False,
                 ref_only_commands=(), inventory=None, none_commands=()):
        self.model = Model(src_root=os.path.abspath(src_root))
        self.cc = compile_commands
        self.ref_only = ref_only_commands
        # TUs built with -DNH_GLOBALS_NONE (before the generated headers
        # exist): they keep plain globals.
        self.none_files = {os.path.abspath(e["file"]) for e in none_commands}
        self.model.none_files = set(self.none_files)
        self.inventory = inventory   # [(name, size, section, object)] of the reference build
        self.exclude = {os.path.abspath(os.path.join(src_root, f)) for f in exclude_files}
        self.index = ci.Index.create()
        self.verbose = verbose
        self.files = {}     # path -> bytes
        self.sysargs = cu.default_clang_args()

    # ------------------------------------------------------------------ util
    def data(self, path):
        d = self.files.get(path)
        if d is None:
            with open(path, "rb") as f:
                d = f.read()
            self.files[path] = d
        return d

    def log(self, *a):
        if self.verbose:
            print(*a, file=sys.stderr)

    # -------------------------------------------------------------- parsing
    def parse(self, entry):
        args = cu.entry_args(entry) + self.sysargs
        tu = self.index.parse(
            entry["file"], args=args,
            options=ci.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD,
        )
        bad = [d for d in tu.diagnostics if d.severity >= ci.Diagnostic.Error]
        for d in bad:
            self.model.diagnostics.append((entry["file"], str(d)))
        return tu

    def run(self):
        m = self.model
        for entry in self.cc:
            main = os.path.abspath(entry["file"])
            if not _in_tree(m, main):
                continue
            if not os.path.exists(main):
                self.log("skip (missing)", main)
                continue
            self.log("analyze", os.path.relpath(main, m.src_root))
            tu = self.parse(entry)
            m.tu_files[main] = {os.path.abspath(i.include.name) for i in tu.get_includes()}
            self.walk_tu(tu, main)
        for entry in self.ref_only:
            main = os.path.abspath(entry["file"])
            if not _in_tree(m, main) or not os.path.exists(main):
                continue
            self.log("analyze (refs only)", os.path.relpath(main, m.src_root))
            tu = self.parse(entry)
            m.tu_files[main] = {os.path.abspath(i.include.name) for i in tu.get_includes()}
            m.ref_only_tus.add(main)
            self.walk_tu(tu, main, refs_only=True)
        self.finish()
        return m

    # ------------------------------------------------------------ AST walk
    def walk_tu(self, tu, main, refs_only=False):
        m = self.model
        excluded = main in self.exclude or refs_only
        stack = []  # enclosing function decls
        # top-level declarator groups keyed by (file, extent start): sibling
        # declarators of one statement share the same extent start.
        self._toplevel = defaultdict(list)
        for c in tu.cursor.get_children():
            if c.kind == CK.VAR_DECL and c.location.file is not None:
                self._toplevel[(os.path.abspath(c.location.file.name), c.extent.start.offset)].append(c)

        # preprocessing record: macro definitions + instantiations
        children = list(tu.cursor.get_children())
        for c in children:
            if c.kind == CK.MACRO_DEFINITION:
                f = c.location.file
                if f is None:
                    continue
                path = os.path.abspath(f.name)
                if not _in_tree(m, path):
                    continue
                m.names_declared[c.spelling].add(("macro", path, ""))
                s, e = c.extent.start.offset, c.extent.end.offset
                if not any(d[0] == path and d[1] == s for d in m.macro_defs[c.spelling]):
                    m.macro_defs[c.spelling].append((path, s, e, self.data(path)[s:e].decode("utf-8", "surrogateescape")))
                if c.spelling not in m.macros:
                    m.macros[c.spelling] = m.macro_defs[c.spelling][0]
            elif c.kind == CK.MACRO_INSTANTIATION:
                f = c.location.file
                if f is None:
                    continue
                path = os.path.abspath(f.name)
                if _in_tree(m, path):
                    key = (path, c.extent.start.offset)
                    if key not in m.inst_at:
                        m.macro_uses[path].append((c.spelling, c.extent.start.offset, c.extent.end.offset))
                        m.inst_at[key] = (c.spelling, c.extent.end.offset)
                        d = c.referenced
                        if d is not None and d.location.file is not None:
                            m.inst_def[key] = os.path.abspath(d.location.file.name)
        for c in children:
            if c.kind in (CK.MACRO_DEFINITION, CK.MACRO_INSTANTIATION, CK.INCLUSION_DIRECTIVE):
                continue
            self.visit(c, main, excluded, stack)

    def visit(self, c, main, excluded, stack):
        m = self.model
        k = c.kind
        if k in NAMED_DECL_KINDS and c.spelling:
            f = c.location.file
            path = os.path.abspath(f.name) if f else "<none>"
            kind = {
                CK.FIELD_DECL: "field", CK.PARM_DECL: "param", CK.VAR_DECL: "var",
                CK.FUNCTION_DECL: "function", CK.ENUM_CONSTANT_DECL: "enumconst",
                CK.TYPEDEF_DECL: "typedef", CK.STRUCT_DECL: "tag", CK.UNION_DECL: "tag",
                CK.ENUM_DECL: "tag", CK.LABEL_STMT: "label", CK.MACRO_DEFINITION: "macro",
            }[k]
            m.names_declared[c.spelling].add((kind, path, c.get_usr()))

        if k == CK.FUNCTION_DECL:
            stack.append(c)
            for ch in c.get_children():
                self.visit(ch, main, excluded, stack)
            stack.pop()
            return

        if k == CK.DECL_STMT:
            group = [ch for ch in c.get_children() if ch.kind == CK.VAR_DECL]
            self._decl_stmt_start = c.extent.start.offset
            for ch in c.get_children():
                if ch.kind == CK.VAR_DECL:
                    if ch.spelling and ch.location.file is not None:
                        m.names_declared[ch.spelling].add(("var", os.path.abspath(ch.location.file.name), ch.get_usr()))
                    self.visit_var(ch, main, excluded, stack, group)
                else:
                    self.visit(ch, main, excluded, stack)
            self._decl_stmt_start = None
            for ch in group:
                for sub in ch.get_children():
                    self.visit(sub, main, excluded, stack)
            return
        if k == CK.VAR_DECL:
            self.visit_var(c, main, excluded, stack)
        elif k == CK.DECL_REF_EXPR:
            self.visit_ref(c, main)

        for ch in c.get_children():
            self.visit(ch, main, excluded, stack)

    # ------------------------------------------------------------ variables
    def visit_var(self, c, main, excluded, stack, group=None):
        m = self.model
        f = c.location.file
        if f is None:
            return
        path = os.path.abspath(f.name)
        if not _in_tree(m, path):
            return
        in_func = stack[-1].spelling if stack else None
        in_func_start = stack[-1].extent.start.offset if stack else None
        sc = c.storage_class
        # Locals that are not static are ordinary automatics: skip.
        if stack and sc not in (SC.STATIC, SC.EXTERN):
            return
        usr = c.get_usr()
        # A file-scope declaration without `extern` is a (possibly tentative)
        # definition; libclang's is_definition() is false for tentative ones.
        is_def = c.is_definition() or sc != SC.EXTERN
        if is_def:
            if usr in m.vars:
                # seen already (the same .c compiled into several targets)
                return
            if excluded:
                return
            if path.endswith((".h",)):
                m.static_in_header.append((path, c.spelling))
                if sc != SC.STATIC:
                    return
                prev = m.header_tu.get(path)
                if prev is not None and prev != main:
                    raise RuntimeError(f"static {c.spelling} defined in header {path} included by several TUs ({prev}, {main})")
                m.header_tu[path] = main
            v = self.make_var(c, path, main, in_func, group)
            v.func_start = in_func_start
            m.vars[usr] = v
            m.by_name[v.name].append(v)
        else:
            # extern declaration (header or in-file)
            stmt_start, stmt_end, names = self.statement_span(c, path, group)
            ed = ExternDecl(path, stmt_start, stmt_end, names, in_func)
            self._pending_externs.setdefault(usr, []).append(ed)

    def make_var(self, c, path, main, in_func, group=None):
        m = self.model
        data = self.data(path)
        stmt_start, stmt_end, names = self.statement_span(c, path, group)
        ext_s, ext_e = c.extent.start.offset, c.extent.end.offset
        name_off = c.location.offset
        toks = tokenize(data, ext_s, stmt_end)
        # locate name token
        ni = next(i for i, t in enumerate(toks) if t.start == name_off)
        # prefix declarator: after last depth-0 ',' before the name, else after specifiers
        prefix_start_i = 0
        depth = 0
        last_comma = None
        for i in range(0, ni):
            t = toks[i].text
            if t in "([{":
                depth += 1
            elif t in ")]}":
                depth -= 1
            elif depth == 0 and t == ",":
                last_comma = i
        if last_comma is not None:
            prefix_start_i = last_comma + 1
        else:
            i = 0
            while i < ni:
                t = toks[i]
                if t.text in ("*", "("):
                    break
                if t.kind == "ident":
                    if t.text == "__attribute__" and toks[i + 1].text == "(":
                        i = find_matching(toks, i + 1, "(", ")") + 1
                        continue
                    if t.text in ("struct", "union", "enum"):
                        i += 1
                        if i < ni and toks[i].kind == "ident" and toks[i].text != "{":
                            i += 1
                        if i < ni and toks[i].text == "{":
                            i = find_matching(toks, i, "{", "}") + 1
                        continue
                i += 1
            prefix_start_i = i
        prefix_toks = toks[prefix_start_i:ni]
        prefix = "".join(_spaced(prefix_toks))
        # suffix / initializer: scan after the name; parens opened in the
        # prefix ('(*name)[N]') close inside the suffix.
        init_start = init_end = None
        init = None
        depth = sum(1 for t in prefix_toks if t.text == "(") - sum(1 for t in prefix_toks if t.text == ")")
        suffix_end_i = len(toks)
        for i in range(ni + 1, len(toks)):
            t = toks[i].text
            if t in "([{":
                depth += 1
            elif t in ")]}":
                depth -= 1
            elif depth == 0 and t in ("=", ",", ";"):
                suffix_end_i = i
                if t == "=":
                    init_start = toks[i + 1].start
                    init_end = ext_e
                    init = data[init_start:init_end].decode("utf-8", "surrogateescape").strip()
                break
        suffix = "".join(_spaced(toks[ni + 1:suffix_end_i]))
        base, chain = cu.base_type(c.type)
        base_spelling = base.spelling
        decl = base.get_declaration()
        base_usr = base_file = None
        bstart = bend = None
        anon = False
        if decl is not None and decl.kind != CK.NO_DECL_FOUND and decl.location.file is not None:
            base_usr = decl.get_usr()
            base_file = os.path.abspath(decl.location.file.name)
            anon = _is_anon(decl)
            defn = decl.get_definition() if decl.kind != CK.TYPEDEF_DECL else decl
            if defn is not None and defn.location.file is not None:
                base_file = os.path.abspath(defn.location.file.name)
                bstart, bend = defn.extent.start.offset, defn.extent.end.offset
            self.note_type(decl)
        # array dimensions from the AST (through pointers: `(*p)[N]`)
        dims = []
        t = c.type
        while True:
            if t.kind == ci.TypeKind.CONSTANTARRAY:
                dims.append(t.element_count)
                t = t.element_type
            elif t.kind == ci.TypeKind.POINTER:
                t = t.get_pointee()
            else:
                break
        # is the declarator text self-contained?
        decl_start = toks[prefix_start_i].start if prefix_start_i < ni else name_off
        decl_end = toks[suffix_end_i - 1].end if suffix_end_i - 1 >= ni else toks[ni].end
        uses = [(mn, a, b) for (mn, a, b) in m.macro_uses.get(path, []) if decl_start <= a < decl_end]
        decl_has_macro = any(a < name_off for (mn, a, b) in uses) or any(
            t.text == "(" for t in toks[ni + 1:suffix_end_i])
        suffix_private = False
        suffix_macro_files = []
        for mn, a, b in uses:
            if a <= name_off:
                continue
            deffile = m.inst_def.get((path, a))
            if deffile is None or not deffile.endswith(".h"):
                suffix_private = True
            else:
                suffix_macro_files.append(deffile)
        self._suffix_ranges.append((path, toks[ni].end, decl_end, c.get_usr()))
        scope = "local" if in_func else ("static" if c.storage_class == SC.STATIC else "global")
        stmt_has_static = any(t.text == "static" for t in toks[:ni])
        v = Var(
            usr=c.get_usr(), name=c.spelling, scope=scope, file=path, func=in_func,
            name_offset=name_off, ext_start=ext_s, ext_end=ext_e,
            stmt_start=stmt_start, stmt_end=stmt_end, stmt_names=names,
            base_spelling=base_spelling, prefix=prefix, suffix=suffix,
            init=init, init_start=init_start, init_end=init_end, ctype=c.type,
            chain=chain, elem_count=(c.type.element_count if c.type.kind == ci.TypeKind.CONSTANTARRAY else None),
            base_decl_usr=base_usr, base_decl_file=base_file, base_is_anon=anon,
            const_object=cu.object_is_const(c.type),
            stmt_has_static_kw=stmt_has_static,
        )
        v.dims = dims
        v.suffix_private = suffix_private
        v.suffix_macro_files = suffix_macro_files
        v.decl_has_macro = decl_has_macro
        v.type_spelling = c.type.spelling
        v.base_decl_start, v.base_decl_end = bstart, bend
        try:
            v.size = c.type.get_size()
            v.align = c.type.get_align()
        except Exception:
            v.size = v.align = 0
        return v

    def statement_span(self, c, path, group=None):
        """Span of the whole declaration statement containing cursor c, plus
        the names of all declarators in it. libclang extents of sibling
        declarators all begin at the type-specifier start."""
        m = self.model
        data = self.data(path)
        start = c.extent.start.offset
        # siblings: same parent, same extent start, VAR_DECL
        parent = c.semantic_parent
        names = []
        end = c.extent.end.offset
        sibs = group if group is not None else self._toplevel.get((path, start), [c])
        start = min([start] + [sib.extent.start.offset for sib in sibs
                               if sib.location.file is not None and os.path.abspath(sib.location.file.name) == path])
        if group is not None and self._decl_stmt_start is not None:
            # block scope: the DeclStmt extent includes `static const ...`
            start = min(start, self._decl_stmt_start)
        if True:
            for sib in sibs:
                if sib.kind == CK.VAR_DECL and sib.location.file is not None and \
                        os.path.abspath(sib.location.file.name) == path:
                    names.append(sib.spelling)
                    end = max(end, sib.extent.end.offset)
        if not names:
            names = [c.spelling]
        # libclang starts the extent after leading macros that expand to
        # nothing (NEARDATA): pull the start back over them.
        ls = data.rfind(b"\n", 0, start) + 1
        lead = data[ls:start]
        if lead.strip():
            covered = [(a, b) for (_, a, b) in m.macro_uses.get(path, []) if ls <= a and b <= start]
            rest = bytearray(lead)
            for a, b in covered:
                rest[a - ls:b - ls] = b" " * (b - a)
            if not rest.strip():
                start = ls
        # skip to ';' (comments may intervene)
        window = 200
        while True:
            toks = tokenize(data, end, min(len(data), end + window))
            hit = next((t for t in toks if t.text == ";"), None)
            if hit is not None:
                end = hit.end
                break
            if end + window >= len(data) or window > 20000:
                raise RuntimeError(f"no ';' after declaration of {c.spelling} in {path}")
            window *= 4
        return start, end, tuple(names)

    def note_type(self, decl):
        """Record a record/enum/typedef declaration that lives in a .c file
        (so it must be hoisted) — with its dependencies."""
        m = self.model
        f = decl.location.file
        if f is None:
            return
        path = os.path.abspath(f.name)
        usr = decl.get_usr()
        if usr in m.types:
            return
        if not _in_tree(m, path):
            return
        defn = decl.get_definition() if decl.kind != CK.TYPEDEF_DECL else decl
        if defn is None:
            defn = decl
        dpath = os.path.abspath(defn.location.file.name)
        kind = {CK.STRUCT_DECL: "struct", CK.UNION_DECL: "union", CK.ENUM_DECL: "enum",
                CK.TYPEDEF_DECL: "typedef"}.get(defn.kind, "other")
        s, e = defn.extent.start.offset, defn.extent.end.offset
        text = self.data(dpath)[s:e].decode("utf-8", "surrogateescape")
        anon = _is_anon(defn)
        td = TypeDef(usr=usr, name="" if anon else defn.spelling, kind=kind, file=dpath, start=s, end=e, text=text,
                     is_definition=defn.is_definition() if defn.kind != CK.TYPEDEF_DECL else True, anon=anon)
        m.types[usr] = td
        # dependencies: type refs inside
        for ch in defn.walk_preorder():
            if ch.kind == CK.DECL_REF_EXPR:
                td.deps_refs.append(ch.spelling)
            if ch.kind == CK.TYPE_REF or ch.kind in (CK.STRUCT_DECL, CK.UNION_DECL, CK.ENUM_DECL, CK.TYPEDEF_DECL):
                if ch.kind == CK.TYPE_REF:
                    ref = ch.referenced
                    if ref is None:
                        continue
                    rf = ref.location.file
                    if rf is None:
                        continue
                    rp = os.path.abspath(rf.name)
                    if rp == dpath and ref.get_usr() != usr:
                        td.deps_types.append(ref.get_usr())
                        self.note_type(ref)
                    elif rp != dpath and _in_tree(m, rp) and rp.endswith(".h"):
                        td.hdr_deps.append(rp)
                    elif rp != dpath and _in_tree(m, rp) and rp.endswith(".c"):
                        td.deps_types.append(ref.get_usr())
                        self.note_type(ref)
            elif ch.kind == CK.FIELD_DECL:
                b, _ = cu.base_type(ch.type)
                bd = b.get_declaration()
                if bd is not None and bd.kind != CK.NO_DECL_FOUND and bd.location.file is not None:
                    bp = os.path.abspath(bd.location.file.name)
                    nested = bp == dpath and s <= bd.extent.start.offset < e
                    if nested:
                        continue
                    if _in_tree(m, bp) and bp.endswith(".c") and bd.get_usr() != usr:
                        td.deps_types.append(bd.get_usr())
                        self.note_type(bd)
                    elif _in_tree(m, bp) and bp.endswith(".h"):
                        td.hdr_deps.append(bp)
        # dependencies: macros used inside the text (resolved later in finish())

    # ------------------------------------------------------------ references
    def visit_ref(self, c, main):
        m = self.model
        ref = c.referenced
        if ref is None or ref.kind not in (CK.VAR_DECL, CK.PARM_DECL):
            return
        usr = ref.get_usr()
        f = c.location.file
        if f is None:
            return
        path = os.path.abspath(f.name)
        off = c.location.offset
        if not _in_tree(m, path):
            return
        name = c.spelling
        key = (path, off)
        if key in m.inst_at:
            # libclang places references that come out of a macro expansion
            # at the macro use site; remember every name resolved there so
            # a macro-body rewrite can be checked for consistency.
            m.name_refs_at[key][name].add(usr)
        if ref.kind != CK.VAR_DECL:
            return
        data = self.data(path)
        in_macro = data[off:off + len(name)] != name.encode()
        self._pending_refs[usr].append(Ref(path, off, path, off, in_macro, main))

    # ------------------------------------------------------------ finish
    _pending_refs = defaultdict(list)
    _pending_externs = {}
    _suffix_ranges = []
    _decl_stmt_start = None

    def finish(self):
        m = self.model
        # attach refs / externs
        for usr, v in m.vars.items():
            seen = set()
            for r in self._pending_refs.get(usr, []):
                key = (r.file, r.offset)
                if key in seen:
                    continue
                seen.add(key)
                v.refs.append(r)
            seen = set()
            for e in self._pending_externs.get(usr, []):
                key = (e.file, e.stmt_start)
                if key in seen:
                    continue
                seen.add(key)
                v.externs.append(e)
        # field names
        used = set()
        for v in m.vars.values():
            fkey = _file_key(os.path.relpath(v.file, m.src_root))
            if v.scope == "global":
                fn = v.name
            elif v.scope == "static":
                fn = f"s_{fkey}_{v.name}"
            else:
                fn = f"l_{fkey}_{v.func}_{v.name}"
            base = fn
            n = 2
            while fn in used:
                fn = f"{base}_{n}"
                n += 1
            used.add(fn)
            v.field_name = fn
        # initializer references to other static-storage objects
        import bisect
        by_file = defaultdict(list)
        for other in m.vars.values():
            for r in other.refs:
                by_file[r.file].append((r.offset, other.usr))
        for f in by_file:
            by_file[f].sort()
        all_init_refs = {}
        for v in m.vars.values():
            if v.init is None:
                continue
            lst = by_file.get(v.file, [])
            i = bisect.bisect_left(lst, (v.init_start, ""))
            refs = set()
            while i < len(lst) and lst[i][0] < v.init_end:
                refs.add(lst[i][1])
                i += 1
            all_init_refs[v.usr] = refs
        # objects the reference compiler already proved read-only (internal
        # linkage, never written: gcc moves them to .rodata/.data.rel.ro)
        # need no migration at all.
        if self.inventory is not None:
            writable = {(o[:-2], n.split(".")[0]) for n, s_, sec, o in self.inventory}
            for v in m.vars.values():
                defobj = os.path.basename(m.header_tu.get(v.file, v.file)) if v.file.endswith(".h") else os.path.basename(v.file)
                if (defobj, v.name) not in writable:
                    v.readonly = True
        # A constant object whose initializer takes the address of a per-env
        # object (`{ 1, &HStealth }`) cannot stay constant: it becomes per-env
        # too (runtime-initialized). Iterate to a fixpoint.
        changed = True
        while changed:
            changed = False
            mig = {u for u, v in m.vars.items() if v.migrate}
            for u, v in m.vars.items():
                if v.migrate or v.keep:
                    continue
                if all_init_refs.get(u, set()) & mig:
                    v.forced = True
                    changed = True
        # all declarators of one definition statement migrate together (so a
        # statement never has to be split)
        groups = defaultdict(list)
        for v in m.vars.values():
            groups[(v.file, v.stmt_start)].append(v)
        for vs in groups.values():
            if any(v.migrate for v in vs):
                for v in vs:
                    if not v.migrate and not v.keep:
                        v.forced = True
        mig = {u for u, v in m.vars.items() if v.migrate}
        for v in m.vars.values():
            v.init_refs = all_init_refs.get(v.usr, set()) & mig
            v.init_has_refs = bool(v.init_refs)
        # array sizes that name other objects (`[sizeof cvt_buf]`)
        for path, a, b, usr in self._suffix_ranges:
            lst = by_file.get(path, [])
            i = bisect.bisect_left(lst, (a, ""))
            if i < len(lst) and lst[i][0] < b:
                m.vars[usr].suffix_private = True
        # macro deps of hoisted types
        for td in m.types.values():
            uses = m.macro_uses.get(td.file, [])
            for name, s, e in uses:
                if td.start <= s < td.end:
                    mac = m.macros.get(name)
                    if mac and mac[0] == td.file:
                        td.deps_macros.append((name, mac[0], mac[1], mac[2], mac[3]))
        # collisions for macro candidates. A global's accessor macro lives in
        # the generated header, i.e. it is visible exactly in the TUs that
        # include hack.h (and in every header, which may be included by
        # one); a declaration in a .c that never includes hack.h cannot
        # clash with it. A static's macro is defined in its own file only.
        hack_h = os.path.join(m.src_root, "include", "hack.h")
        hack_tus = {tu for tu, incs in m.tu_files.items() if hack_h in incs}
        for v in m.vars.values():
            if not v.migrate:
                continue
            if v.scope == "local":
                v.access = "rewrite"
                continue
            decls = m.names_declared.get(v.name, set())
            coll = []
            own_tu = m.header_tu.get(v.file, v.file)
            own_files = m.tu_files.get(own_tu, set()) | {own_tu, v.file}
            for kind, path, usr in sorted(decls):
                if usr == v.usr:
                    continue
                if v.scope == "static" and path not in own_files:
                    continue
                if v.scope == "global" and not path.endswith(".h") and path not in hack_tus:
                    continue
                coll.append((kind, os.path.relpath(path, m.src_root) if _in_tree(m, path) else path, usr))
            if coll:
                v.collisions = coll
                v.access = "rewrite"


def _is_anon(decl):
    sp = decl.spelling or ""
    if hasattr(decl, "is_anonymous") and decl.kind in (CK.STRUCT_DECL, CK.UNION_DECL, CK.ENUM_DECL):
        try:
            if decl.is_anonymous():
                return True
        except Exception:
            pass
    return sp == "" or "(" in sp or "unnamed" in sp or "anonymous" in sp


def _same_object(usr, v):
    return usr == v.usr


def _file_key(rel):
    base = os.path.basename(rel)
    return base.replace(".", "_").replace("-", "_")


def _spaced(toks):
    """Join tokens with minimal spacing preserving identifiers separation."""
    out = []
    prev = None
    for t in toks:
        if prev is not None and (prev.kind in ("ident", "num") and t.kind in ("ident", "num")):
            out.append(" ")
        out.append(t.text)
        prev = t
    return out
