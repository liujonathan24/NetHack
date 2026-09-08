"""Command line driver.

  python -m nhglobals analyze --src <tree> --build <builddir> [--exclude f]...
  python -m nhglobals apply   --src <tree> --build <builddir> [...]
  python -m nhglobals inventory <objdir-or-lib>
"""
from __future__ import annotations

import argparse
import os
import pickle
import subprocess
import sys
from collections import Counter, defaultdict

def _analyzer(args):
    from . import clangutil as cu
    from .analyze import Analyzer
    allcc = cu.load_compile_commands(os.path.join(args.build, "compile_commands.json"))
    cmd = lambda e: e.get("command") or " ".join(e.get("arguments", []))
    cc = [e for e in allcc if f"/{args.target}.dir/" in cmd(e)]
    reft = list(args.ref_target or []) + list(args.none_target or [])
    refcc = [e for e in allcc if any(f"/{t}.dir/" in cmd(e) for t in reft)]
    nonecc = [e for e in allcc if any(f"/{t}.dir/" in cmd(e) for t in (args.none_target or []))]
    exclude = list(args.exclude or [])
    inv = inventory(args.inventory) if args.inventory else None
    return Analyzer(args.src, cc, exclude_files=exclude, verbose=args.verbose,
                    ref_only_commands=refcc, inventory=inv, none_commands=nonecc)


def cmd_analyze(args):
    an = _analyzer(args)
    m = an.run()
    report(m, args)
    if args.pickle:
        with open(args.pickle, "wb") as f:
            for v in m.vars.values():
                v.ctype = None
            pickle.dump(m, f)


def report(m, args):
    rel = lambda p: os.path.relpath(p, m.src_root) if p and p.startswith(m.src_root) else p
    print(f"source root: {m.src_root}")
    if m.diagnostics:
        print(f"!! {len(m.diagnostics)} parse errors:")
        for f, d in m.diagnostics[:20]:
            print("   ", rel(f), d)
    keep = [v for v in m.vars.values() if v.keep]
    const = [v for v in m.vars.values() if v.const_object]
    ro = [v for v in m.vars.values() if v.readonly and not v.const_object]
    mig = [v for v in m.vars.values() if v.migrate]
    print(f"  read-only per reference build (left alone): {len(ro)}")
    print(f"  const objects forced per-env (initializer takes a per-env address): {len([v for v in mig if v.forced])}")
    print(f"static-storage definitions found: {len(m.vars)}")
    print(f"  const objects (stay in .rodata): {len(const)}")
    print(f"  to migrate: {len(mig)}  by scope: {Counter(v.scope for v in mig)}")
    print(f"  access: {Counter(v.access for v in mig)}")
    print(f"  with initializer: {sum(1 for v in mig if v.init is not None)}"
          f"  (of which referencing migrated vars: {sum(1 for v in mig if v.init_has_refs)})")
    print(f"  refs total: {sum(len(v.refs) for v in mig)}  in-macro refs: {sum(1 for v in mig for r in v.refs if r.in_macro)}")
    if m.static_in_header:
        print(f"  static definitions in headers: {m.static_in_header[:10]}")
    coll = [v for v in mig if v.collisions]
    print(f"  macro-name collisions -> precise rewrite: {len(coll)}")
    for v in sorted(coll, key=lambda v: -len(v.refs))[: args.show]:
        kinds = Counter(k for k, _, _ in v.collisions)
        print(f"    {v.name:28s} {v.scope:6s} refs={len(v.refs):5d} {dict(kinds)} e.g. {v.collisions[0][1]}")
    hoist = {v.base_decl_usr for v in mig if v.base_decl_file and v.base_decl_file.endswith(".c")}
    print(f"  types to hoist from .c files: {len(hoist)}")
    for u in list(hoist)[: args.show]:
        td = m.types.get(u)
        if td:
            print(f"    {td.kind} {td.name or '<anon>'} @ {rel(td.file)} macros={[d[0] for d in td.deps_macros]} deps={len(td.deps_types)} hdrs={[rel(h) for h in td.hdr_deps]}")
    # header deps of field types outside hack.h
    hdrs = Counter()
    for v in mig:
        if v.base_decl_file and v.base_decl_file.endswith(".h"):
            hdrs[rel(v.base_decl_file)] += 1
    print(f"  header files defining field types: {dict(hdrs)}")
    multi = [v for v in mig if len(v.stmt_names) > 1]
    print(f"  multi-declarator statements: {len(multi)}")
    fp = [v for v in mig if "(" in v.prefix]
    print(f"  function-pointer-ish declarators: {len(fp)} e.g. {[(v.name, v.prefix, v.suffix) for v in fp[:5]]}")
    if args.inventory:
        cross_check(m, args.inventory)
    if args.show_all:
        for v in sorted(mig, key=lambda v: (v.file, v.name_offset)):
            print(f"  {v.scope:6s} {rel(v.file):28s} {v.func or '':24s} {v.base_spelling} {v.prefix}{v.name}{v.suffix}"
                  f"{' = ' + v.init[:40].replace(chr(10), ' ') if v.init else ''}   refs={len(v.refs)} -> {v.field_name} [{v.access}]")


def cross_check(m, objdir):
    """Every writable symbol of the compiled objects must correspond to a
    migrated variable, and vice versa. Prints the differences."""
    rel = lambda p: os.path.relpath(p, m.src_root)
    inv = inventory(objdir)
    found = defaultdict(list)
    for v in m.vars.values():
        found[os.path.basename(v.file)].append(v)
    print(f"cross-check against {len(inv)} writable symbols in {objdir}")
    unmatched = []
    for name, size, sec, obj in inv:
        base = name.split(".")[0]
        src = obj[:-2]
        cands = [v for v in found.get(src, []) if v.name == base]
        if not cands:
            unmatched.append((obj, name, size, sec, "NOT FOUND"))
        for v in cands:
            if v.const_object:
                unmatched.append((obj, name, size, sec, "const object in writable section"))
            elif v.keep:
                unmatched.append((obj, name, size, sec, "kept (whitelisted)"))
    print(f"  inventory symbols without a migrated variable: {len([u for u in unmatched if u[4] == 'NOT FOUND'])}")
    for u in unmatched:
        print("   ", *u)
    invnames = {(o[:-2], n.split(".")[0]) for n, s, sec, o in inv}
    extra = [v for v in m.vars.values() if not v.const_object and (os.path.basename(v.file), v.name) not in invnames]
    print(f"  migrated variables with no writable symbol (optimized away / const-folded): {len(extra)}")
    for v in extra:
        print("   ", rel(v.file), v.scope, v.name, v.func or "")
    return unmatched, extra


def cmd_inventory(args):
    for line in inventory(args.path):
        print("\t".join(str(x) for x in line))


def inventory(path):
    """Writable symbols (name, size, section, object) from a library or a
    directory of object files."""
    out = []
    objs = []
    if os.path.isdir(path):
        for root, _, files in os.walk(path):
            for f in files:
                if f.endswith(".o"):
                    objs.append(os.path.join(root, f))
    else:
        objs.append(path)
    import re
    pat = re.compile(r"^([0-9a-f]+)\s+(.{7})\s+(\S+)\s+([0-9a-f]+)\s+(?:\.hidden\s+|\.protected\s+|\.internal\s+)?(\S+)$")
    for o in sorted(objs):
        txt = subprocess.run(["objdump", "-t", o], capture_output=True, text=True, check=True).stdout
        for line in txt.splitlines():
            mm = pat.match(line)
            if not mm:
                continue
            _, flags, sec, size, name = mm.groups()
            if name.startswith(".") or name in ("__TMC_END__", "__dso_handle") or name.startswith("completed."):
                continue
            writable = sec in (".bss", "*COM*", ".tbss", ".tdata") or (sec.startswith(".data") and not sec.startswith(".data.rel.ro"))
            if not writable:
                continue
            out.append((name, int(size, 16), sec, os.path.basename(o)))
    return out


def main(argv=None):
    ap = argparse.ArgumentParser(prog="nhglobals")
    sub = ap.add_subparsers(dest="cmd", required=True)

    def common(p):
        p.add_argument("--src", required=True, help="source tree root (contains src/, include/)")
        p.add_argument("--build", required=True, help="cmake build dir with compile_commands.json")
        p.add_argument("--target", default="nethack", help="only TUs of this cmake target")
        p.add_argument("--ref-target", action="append", help="cmake target whose TUs only get their references rewritten (util programs)")
        p.add_argument("--none-target", action="append", help="cmake target built with -DNH_GLOBALS_NONE (plain globals; e.g. makedefs, which runs before the generated headers exist)")
        p.add_argument("--const-promote", action="append", default=[], help="global that is never written: make it const instead of per-env")
        p.add_argument("--force", action="append", default=[], help="migrate this object even if the reference build placed it in a read-only section")
        p.add_argument("--inventory", help="object dir of the unmodified build: only symbols writable there are migrated")
        p.add_argument("--exclude", action="append", help="source file (relative to --src) left untouched")
        p.add_argument("--keep", action="append", default=[], help="symbol to leave as a process global")
        p.add_argument("--keep-file", help="file with one symbol per line to keep (# comments)")
        p.add_argument("-v", "--verbose", action="store_true")

    p = sub.add_parser("analyze")
    common(p)
    p.add_argument("--show", type=int, default=15)
    p.add_argument("--show-all", action="store_true")
    p.add_argument("--pickle")
    p.set_defaults(func=cmd_analyze)

    p = sub.add_parser("apply")
    common(p)
    p.add_argument("--dry-run", action="store_true")
    p.add_argument("--model", help="cache the analysis in this pickle (reuse if it exists)")
    p.add_argument("--pointer", default="nh_g")
    p.add_argument("--struct", default="nh_globals")
    p.set_defaults(func=cmd_apply)

    p = sub.add_parser("inventory")
    p.add_argument("path")
    p.set_defaults(func=cmd_inventory)

    args = ap.parse_args(argv)
    return args.func(args)


def cmd_apply(args):
    from .rewrite import apply_all
    an = _analyzer(args)
    if args.model and os.path.exists(args.model):
        with open(args.model, "rb") as f:
            m = pickle.load(f)
        an.model = m
    else:
        m = an.run()
        if args.model:
            for v in m.vars.values():
                v.ctype = None
            with open(args.model, "wb") as f:
                pickle.dump(m, f)
    keep = set(args.keep)
    if args.keep_file:
        with open(args.keep_file) as f:
            for line in f:
                line = line.split("#", 1)[0].strip()
                if line:
                    keep.add(line)
    for v in m.vars.values():
        if v.name in keep:
            v.keep = True
        if v.name in set(args.const_promote) and v.scope == "global":
            v.const_promote = True
        if v.name in set(args.force):
            v.readonly = False
            v.forced = True
    if m.diagnostics:
        print(f"refusing to rewrite: {len(m.diagnostics)} parse errors", file=sys.stderr)
        for f, d in m.diagnostics[:20]:
            print("   ", f, d, file=sys.stderr)
        return 1
    apply_all(m, an, pointer=args.pointer, struct=args.struct, dry_run=args.dry_run)
    return 0


if __name__ == "__main__":
    sys.exit(main())
