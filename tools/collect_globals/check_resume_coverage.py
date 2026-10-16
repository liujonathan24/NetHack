#!/usr/bin/env python3
"""Every field of struct nh_globals is on the resume path or has a stated reason.

A cross-process resume loads the level blob, the player blob (savegamestate
plus the nle_player_tail), the RNG blob and the mirror blob. Every per-game
global has to be in one of them, be rebuilt by the load, or be explained. The
class of each field is decided from the sources, in this order:

  saved      the field is used by a save function (SAVE_FUNCTIONS)
  tail       the field is written by nle_save_player_tail (save.c)
  rebuilt    listed in resume_rebuilt.txt with the function that restores it
  allowlist  listed in resume_allowlist.txt with a class and a reason

A field in none of them fails the gate, as does a stale line in either list,
a tail struct field with no global behind it, and an audit "dropped-play"
field that is no longer carried.

  tools/collect_globals/check_resume_coverage.py [-v] [--root DIR]
"""
import argparse
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))

# every function that writes game state into a blob: the player blob
# (savegamestate and what it calls), the level blob (savelev and what it
# calls), the RNG blob (nle_rng_save) and the mirror blob (nle_mirror_save)
SAVE_FUNCTIONS = [
    "savegamestate", "save_killers", "save_timers", "save_light_sources",
    "saveobjchn", "saveobj", "savemonchn", "savemon", "savetrapchn",
    "savefruitchn", "savelevchn", "savecemetery", "savedamage",
    "save_dungeon", "save_mapseen", "savenames", "save_artifacts",
    "save_oracles", "save_waterlevel", "save_msghistory",
    "tty_getmsghistory", "msghistory_snapshot",
    "savelev", "save_rooms", "save_engravings", "save_regions", "save_worm",
    "nle_rng_save", "nle_mirror_save",
]
TAIL_FUNCTION = "nle_save_player_tail"
TAIL_CALLER = "nle_save_player_inner"
TAIL_STRUCT = "nle_player_tail"

ALLOWLIST_CLASSES = ("constant", "process", "options", "display",
                     "transient", "prompt-only")

# audit/resume.md section 3 (2026-10-14): the fields the on-disk checkpoint
# dropped that can change play. Each must now be in a blob, the tail or
# rebuilt. The tune knobs live in nle_ctx_t, outside struct nh_globals; the
# engine's checkpoint JSON carries them (tests/test_resume.py).
DROPPED_PLAY = [
    "s_rnd_c_rnglist", "s_invent_c_lastinvnr", "s_track_c_utrack",
    "s_track_c_utcnt", "s_track_c_utpnt", "s_makemon_c_rndmonst_state",
    "l_uhitm_c_hitum_cleave_clockwise", "l_mkmaze_c_movebubbles_up",
    "youmonst.movement", "youmonst.mspec_used", "iflags.travelcc",
]

SOURCE_DIRS = ("src", "include", "win", "sys")

_STRIP = re.compile(r'/\*.*?\*/|//[^\n]*|"(?:\\.|[^"\\\n])*"|\'(?:\\.|[^\'\\\n])*\'',
                    re.S)
_IDENT = re.compile(r"(?<![\w.])(?<!->)\b([A-Za-z_]\w*)\b")
_NH_G = re.compile(r"\bNH_G\s*\(\s*(\w+)\s*\)|\bnh_g\s*->\s*(\w+)")
_ALIAS = re.compile(r"^\s*#\s*define\s+(\w+)\s+\(nh_g->(\w+)\)", re.M)


def strip_c(text):
    """Comments and string/char literals replaced by blanks."""
    return _STRIP.sub(lambda m: " " if m.group(0)[0] == "/" else '""', text)


# platforms this library is never built for: their #ifdef blocks are dropped
DEAD_PLATFORMS = ("MFLOPPY", "TOS", "MAC", "VMS", "MSDOS", "WIN32", "OS2",
                  "AMIGA", "AMII_GRAPHICS", "MICRO")


def drop_dead_ifdefs(text):
    """text without the #ifdef blocks of DEAD_PLATFORMS (their #else kept)."""
    out, stack = [], []  # per open #if: "drop", "drop-else" or "keep"
    for line in text.split("\n"):
        m = re.match(r"\s*#\s*(ifdef|ifndef|if|elif|else|endif)\b\s*(\w*)", line)
        if m:
            kw, name = m.groups()
            if kw in ("if", "ifdef", "ifndef"):
                dead = kw != "if" and name in DEAD_PLATFORMS
                stack.append("keep" if not dead else "drop" if kw == "ifdef" else "drop-else")
            elif kw in ("else", "elif") and stack:
                stack[-1] = {"drop": "keep", "drop-else": "drop", "keep": "keep"}[stack[-1]]
            elif kw == "endif" and stack:
                stack.pop()
            out.append("")
            continue
        out.append("" if "drop" in stack else line)
    return "\n".join(out)


def assignments(body):
    """[(name, rhs)] of every `name = rhs` in body (declaration lists too)."""
    out = []
    for m in re.finditer(r"(?<![\w.])(?<!->)(\w+)\s*=(?![=])", body):
        depth, i = 0, m.end()
        while i < len(body) and (depth or body[i] not in ";,"):
            depth += {"(": 1, ")": -1, "[": 1, "]": -1}.get(body[i], 0)
            i += 1
        out.append((m.group(1), body[m.end():i]))
    return out


class Sources:
    def __init__(self, root):
        self.root = root
        self.files = {}
        base = os.path.join(root, "src")
        for d in SOURCE_DIRS:
            for dirpath, _, names in os.walk(os.path.join(base, d)):
                reldir = os.path.relpath(dirpath, base)
                if reldir.startswith(("build", "third_party")) or os.sep + "build" in reldir:
                    continue
                for n in names:
                    if n.endswith((".c", ".h", ".cc")):
                        p = os.path.join(dirpath, n)
                        rel = os.path.relpath(p, base)
                        with open(p, encoding="utf-8", errors="replace") as f:
                            self.files[rel] = f.read()
        self.header_alias = {}
        self.file_alias = {}
        for rel, text in self.files.items():
            for name, field in _ALIAS.findall(text):
                if rel.endswith(".h"):
                    self.header_alias[name] = field
                else:
                    self.file_alias.setdefault(rel, {})[name] = field

    def text(self, rel):
        return self.files[rel]

    def definitions(self, fn):
        """[(rel, body)] of every definition of fn (K&R style: name at column 0)."""
        out = []
        pat = re.compile(r"^%s\s*\(" % re.escape(fn), re.M)
        for rel, text in self.files.items():
            if not rel.endswith((".c", ".cc")):
                continue
            text = drop_dead_ifdefs(text)
            for m in pat.finditer(text):
                start = text.find("\n{", m.end())
                end = text.find("\n}", start)
                if start < 0 or end < 0:
                    continue
                out.append((rel, strip_c(text[start + 2:end])))
        return out

    def resolve(self, rel, name):
        return self.file_alias.get(rel, {}).get(name) or self.header_alias.get(name)

    def refs(self, rel, body, fields):
        """Fields of struct nh_globals that body (from file rel) refers to."""
        found = set()
        for a, b in _NH_G.findall(body):
            if (a or b) in fields:
                found.add(a or b)
        for tok in _IDENT.findall(body):
            f = self.resolve(rel, tok)
            if f in fields:
                found.add(f)
        return found


def parse_struct(text, name):
    """[(field, orig, file, line)] of struct `name` from a generated header.

    The generator leaves one declaration per line (a `/* orig */` comment with
    the original identifier at its end) and groups them under
    `/* ---- file ---- */` separators.
    """
    lines = text.split("\n")
    start = next(i for i, l in enumerate(lines) if l.startswith("struct %s {" % name))
    out, cur_file, buf = [], None, ""
    for i in range(start + 1, len(lines)):
        l = lines[i]
        if l.startswith("};"):
            break
        m = re.match(r"\s*/\* ---- (\S+) ---- \*/", l)
        if m:
            cur_file = m.group(1)
            continue
        s = l.strip()
        if not s or s.startswith("/*") or s.startswith("#"):
            continue
        buf += " " + s
        if not s.endswith(";") and not (s.endswith("*/") and ";" in s):
            continue
        decl = buf.strip()
        buf = ""
        cm = re.search(r"/\*\s*(.*?)\s*\*/\s*$", decl)
        comment = cm.group(1) if cm else ""
        body = (decl[:cm.start()] if cm else decl).strip().rstrip(";").strip()
        body = re.sub(r"__attribute__\s*\(\(.*\)\)", "", body).strip()
        fp = re.search(r"\(\*\s*([A-Za-z_]\w*)\s*\)", body)
        if fp:
            field = fp.group(1)
        else:
            field = re.search(r"([A-Za-z_]\w*)\s*((\[[^\]]*\]\s*)*)$", body).group(1)
        orig = comment.split()[-1] if comment else field
        out.append((field, orig, cur_file, i + 1))
    return out


def parse_tail_struct(text):
    """Field names of struct nle_player_tail (nle.h)."""
    m = re.search(r"struct %s \{(.*?)\n\};" % TAIL_STRUCT, text, re.S)
    names = []
    for decl in strip_c(m.group(1)).split(";"):
        for p in re.sub(r"\[[^\]]*\]", "", decl).split(","):
            if p.strip():
                names.append(re.findall(r"\w+", p)[-1])
    return names


def read_list(path, ncols):
    """[(line_no, cols)] of a tab-separated list file with '#' comments."""
    out = []
    with open(path, encoding="utf-8") as f:
        for n, line in enumerate(f, 1):
            line = line.rstrip("\n")
            if not line.strip() or line.lstrip().startswith("#"):
                continue
            cols = [c.strip() for c in line.split("\t")]
            cols = [c for c in cols if c]
            if len(cols) != ncols:
                out.append((n, None))
            else:
                out.append((n, cols))
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--root", default=os.path.normpath(os.path.join(HERE, "..", "..")))
    ap.add_argument("--allowlist", default=os.path.join(HERE, "resume_allowlist.txt"))
    ap.add_argument("--rebuilt", default=os.path.join(HERE, "resume_rebuilt.txt"))
    ap.add_argument("-v", "--verbose", action="store_true")
    args = ap.parse_args()

    src = Sources(args.root)
    struct_fields = parse_struct(src.text("include/nh_globals.h"), "nh_globals")
    fields = {f for f, _, _, _ in struct_fields}
    where = {f: (orig, fil, line) for f, orig, fil, line in struct_fields}
    problems = []

    # 1. the save functions
    saved = {}
    for fn in SAVE_FUNCTIONS:
        defs = src.definitions(fn)
        if not defs:
            problems.append("save function %s() is not defined anywhere" % fn)
        for rel, body in defs:
            for f in src.refs(rel, body, fields):
                saved.setdefault(f, []).append(fn)

    # 2. the tail
    tail_defs = src.definitions(TAIL_FUNCTION)
    caller_defs = src.definitions(TAIL_CALLER)
    if len(tail_defs) != 1 or len(caller_defs) != 1:
        problems.append("%s/%s: expected one definition each" % (TAIL_FUNCTION, TAIL_CALLER))
        tail_rel, tail_body = "src/save.c", ""
        caller_body = ""
    else:
        (tail_rel, tail_body), (_, caller_body) = tail_defs[0], caller_defs[0]
    tail = src.refs(tail_rel, tail_body, fields)

    locals_ = {}
    for name, rhs in assignments(caller_body + tail_body):
        locals_[name] = locals_.get(name, "") + " " + rhs

    def resolve_rhs(rhs, seen=frozenset()):
        """Fields behind rhs, through the locals it names."""
        found = src.refs(tail_rel, rhs, fields)
        for tok in _IDENT.findall(rhs):
            if tok in locals_ and tok not in seen:
                found |= resolve_rhs(locals_[tok], seen | {tok})
        return found

    tail_struct = parse_tail_struct(src.text("include/nle.h"))
    for tf in tail_struct:
        rhss = re.findall(r"\bt\.%s\s*=\s*([^;]+);" % tf, tail_body)
        rhss += re.findall(r"memcpy\s*\(\s*t\.%s\s*,\s*([^,]+)," % tf, tail_body)
        if not rhss:
            problems.append("tail field %s is never written by %s()" % (tf, TAIL_FUNCTION))
            continue
        got = set()
        for rhs in rhss:
            got |= resolve_rhs(rhs)
        if not got:
            problems.append("tail field %s has no matching global (written from: %s)"
                            % (tf, "; ".join(r.strip() for r in rhss)))
        tail |= got
    for part in DROPPED_PLAY:
        if "." in part and part not in tail_body:
            problems.append("audit dropped-play member %s is not written by %s()" % (part, TAIL_FUNCTION))

    # 3. the rebuilt list and the allowlist
    rebuilt, allow = {}, {}
    for n, cols in read_list(args.rebuilt, 3):
        if cols is None:
            problems.append("%s:%d: expected field<TAB>function<TAB>reason" % (args.rebuilt, n))
            continue
        f, fn, reason = cols
        if f not in fields:
            problems.append("%s:%d: %s is not a field of struct nh_globals" % (args.rebuilt, n, f))
        elif f in rebuilt:
            problems.append("%s:%d: %s listed twice" % (args.rebuilt, n, f))
        elif f in saved or f in tail:
            problems.append("%s:%d: %s is already %s" % (args.rebuilt, n, f, "saved" if f in saved else "in the tail"))
        elif not src.definitions(fn):
            problems.append("%s:%d: %s: no function %s() in the sources" % (args.rebuilt, n, f, fn))
        else:
            rebuilt[f] = (fn, reason)
    for n, cols in read_list(args.allowlist, 3):
        if cols is None:
            problems.append("%s:%d: expected field<TAB>class<TAB>reason" % (args.allowlist, n))
            continue
        f, cls, reason = cols
        if f not in fields:
            problems.append("%s:%d: %s is not a field of struct nh_globals" % (args.allowlist, n, f))
        elif cls not in ALLOWLIST_CLASSES:
            problems.append("%s:%d: %s: class %r is not one of %s" % (args.allowlist, n, f, cls, ", ".join(ALLOWLIST_CLASSES)))
        elif f in allow:
            problems.append("%s:%d: %s listed twice" % (args.allowlist, n, f))
        elif f in saved or f in tail or f in rebuilt:
            problems.append("%s:%d: %s is already %s" % (args.allowlist, n, f,
                            "saved" if f in saved else "in the tail" if f in tail else "rebuilt"))
        else:
            allow[f] = (cls, reason)

    # 4. every field has a class
    classes = {}
    for f in fields:
        if f in saved:
            classes[f] = "saved"
        elif f in tail:
            classes[f] = "tail"
        elif f in rebuilt:
            classes[f] = "rebuilt"
        elif f in allow:
            classes[f] = allow[f][0]
        else:
            orig, fil, line = where[f]
            problems.append("%s (%s, %s) nh_globals.h:%d is in no category: save it, "
                            "name what rebuilds it in resume_rebuilt.txt, or explain it in "
                            "resume_allowlist.txt" % (f, orig, fil, line))
    for f in DROPPED_PLAY:
        if "." not in f and classes.get(f) not in ("saved", "tail", "rebuilt"):
            problems.append("audit dropped-play field %s is %s, not in a blob, the tail or rebuilt"
                            % (f, classes.get(f, "unclassified")))

    # report
    counts = {}
    for c in classes.values():
        counts[c] = counts.get(c, 0) + 1
    order = ["saved", "tail", "rebuilt"] + list(ALLOWLIST_CLASSES)
    print("struct nh_globals: %d fields; tail: %d struct fields" % (len(fields), len(tail_struct)))
    for c in order:
        if counts.get(c):
            print("  %-12s %4d" % (c, counts[c]))
    if args.verbose:
        by_fn = {}
        for f, fns in saved.items():
            for fn in fns:
                by_fn.setdefault(fn, []).append(f)
        for fn in SAVE_FUNCTIONS:
            if fn in by_fn:
                print("  %s: %s" % (fn, ", ".join(sorted(set(by_fn[fn])))))
        print("  tail: %s" % ", ".join(sorted(tail)))
        for f, (fn, reason) in sorted(rebuilt.items()):
            print("  rebuilt %s by %s: %s" % (f, fn, reason))
    prompt_only = sorted(f for f, (c, _) in allow.items() if c == "prompt-only")
    print("prompt-only (a checkpoint is refused while they can be live): %d" % len(prompt_only))
    for f in prompt_only:
        print("  %s: %s" % (f, allow[f][1]))
    if problems:
        print("FAIL: %d problem(s)" % len(problems))
        for p in problems:
            print("  " + p)
        return 1
    print("OK: every field of struct nh_globals is saved, in the tail, rebuilt or allowlisted")
    return 0


if __name__ == "__main__":
    sys.exit(main())
