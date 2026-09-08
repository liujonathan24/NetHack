"""Thin helpers over the libclang Python binding.

Everything the collector needs from libclang that the stock binding does not
expose directly (spelling locations, raw byte offsets) lives here.
"""
from __future__ import annotations

import ctypes
import json
import os
import shlex
from ctypes import POINTER, byref, c_uint

import clang.cindex as ci

_lib = ci.conf.lib
_lib.clang_getSpellingLocation.argtypes = [
    ci.SourceLocation,
    POINTER(ci.c_object_p),
    POINTER(c_uint),
    POINTER(c_uint),
    POINTER(c_uint),
]
_lib.clang_getSpellingLocation.restype = None


def spelling_location(loc: ci.SourceLocation):
    """Return (filename, byte offset) of where a token was actually spelled.

    For a token that comes out of a macro expansion this is the position inside
    the macro definition (or the macro argument at the use site); for ordinary
    tokens it equals the expansion location.
    """
    f = ci.c_object_p()
    line = c_uint()
    col = c_uint()
    off = c_uint()
    _lib.clang_getSpellingLocation(loc, byref(f), byref(line), byref(col), byref(off))
    if not f:
        return None, None
    fobj = ci.File(f)
    return fobj.name, int(off.value)


def expansion_location(loc: ci.SourceLocation):
    f = loc.file
    return (f.name if f else None), loc.offset


def default_clang_args(extra_isystem=None):
    """System include dirs so libclang (no bundled resource dir) finds
    stddef.h & co. We reuse the host gcc's private include directory."""
    args = []
    cands = []
    if extra_isystem:
        cands.extend(extra_isystem)
    # host gcc private headers
    for root in ("/usr/lib/gcc/x86_64-linux-gnu", "/usr/lib/gcc/aarch64-linux-gnu"):
        if os.path.isdir(root):
            vers = sorted(os.listdir(root), key=lambda v: [int(x) if x.isdigit() else 0 for x in v.split(".")])
            for v in reversed(vers):
                inc = os.path.join(root, v, "include")
                if os.path.isdir(inc):
                    cands.append(inc)
                    break
    for c in cands:
        args += ["-isystem", c]
    return args


def load_compile_commands(path: str):
    with open(path) as f:
        return json.load(f)


def entry_args(entry):
    """Compiler args for libclang from a compile_commands entry (drops the
    compiler, -c/-o and the source file)."""
    if "arguments" in entry:
        argv = list(entry["arguments"])
    else:
        argv = shlex.split(entry["command"])
    out = []
    skip = False
    for a in argv[1:]:
        if skip:
            skip = False
            continue
        if a in ("-c",):
            continue
        if a == "-o":
            skip = True
            continue
        if a.startswith("-o") and len(a) > 2 and a.endswith(".o"):
            continue
        if a == entry["file"] or a.endswith(("/" + os.path.basename(entry["file"]))):
            continue
        out.append(a)
    return out


class SourceFile:
    """A file's bytes plus an edit list applied at the end."""

    def __init__(self, path: str):
        self.path = path
        with open(path, "rb") as f:
            self.data = f.read()
        self.edits = []  # (start, end, replacement bytes)

    def text(self, start: int, end: int) -> str:
        return self.data[start:end].decode("utf-8", "surrogateescape")

    def replace(self, start: int, end: int, new: str):
        self.edits.append((start, end, new.encode("utf-8", "surrogateescape")))

    def insert(self, at: int, new: str):
        self.edits.append((at, at, new.encode("utf-8", "surrogateescape")))

    def line_start(self, off: int) -> int:
        i = self.data.rfind(b"\n", 0, off)
        return i + 1

    def line_end(self, off: int) -> int:
        i = self.data.find(b"\n", off)
        return len(self.data) if i < 0 else i + 1

    def apply(self) -> bytes:
        # sort by start; verify no overlapping non-insert edits
        edits = sorted(self.edits, key=lambda e: (e[0], e[1]))
        out = bytearray()
        pos = 0
        last_end = -1
        for s, e, new in edits:
            if s < pos:
                if s == e and s == last_end:
                    # insertion exactly at previous edit end: fine
                    pass
                else:
                    prev = [x for x in edits if x[1] == last_end][-1]
                    raise RuntimeError(f"overlapping edits in {self.path} at {s}..{e} (pos {pos}):\n"
                                       f"  previous {prev[0]}..{prev[1]}: {self.data[prev[0]:prev[1]][:120]!r} -> {prev[2][:80]!r}\n"
                                       f"  this     {s}..{e}: {self.data[s:e][:120]!r} -> {new[:80]!r}")
            out += self.data[pos:s]
            out += new
            pos = e
            last_end = e
        out += self.data[pos:]
        return bytes(out)

    def write(self):
        new = self.apply()
        with open(self.path, "wb") as f:
            f.write(new)
        self.data = new
        self.edits = []


def usr(cursor: ci.Cursor) -> str:
    return cursor.get_usr()


def base_type(t: ci.Type):
    """Peel arrays / pointers / function types down to the base object type.
    Returns (base, chain) where chain is a list of ('ptr'|'arr'|'fn') steps."""
    chain = []
    while True:
        k = t.kind
        if k in (ci.TypeKind.CONSTANTARRAY, ci.TypeKind.INCOMPLETEARRAY, ci.TypeKind.VARIABLEARRAY):
            chain.append("arr")
            t = t.element_type
        elif k == ci.TypeKind.POINTER:
            chain.append("ptr")
            t = t.get_pointee()
        elif k in (ci.TypeKind.FUNCTIONPROTO, ci.TypeKind.FUNCTIONNOPROTO):
            chain.append("fn")
            t = t.get_result()
        else:
            return t, chain


def object_is_const(t: ci.Type) -> bool:
    """Is the *object* itself immutable (so it may live in .rodata)?
    `const char *p` is NOT (the pointer is writable); `const char *const p`
    and `const int tbl[]` are."""
    k = t.kind
    if k in (ci.TypeKind.CONSTANTARRAY, ci.TypeKind.INCOMPLETEARRAY):
        return object_is_const(t.element_type)
    return t.is_const_qualified()
