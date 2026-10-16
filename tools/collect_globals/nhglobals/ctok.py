"""A small C tokenizer over a byte range of a file.

Only what the collector needs: identifiers, numbers, string/char literals,
punctuators, with comments and whitespace skipped. Offsets are byte offsets
into the file, matching libclang.
"""
from __future__ import annotations

import re
from dataclasses import dataclass

_TOK = re.compile(
    rb"(?P<ws>(?:\s|\\\r?\n)+)"
    rb"|(?P<lcomment>//[^\n]*)"
    rb"|(?P<bcomment>/\*.*?\*/)"
    rb"|(?P<str>\"(?:\\.|[^\"\\])*\")"
    rb"|(?P<chr>'(?:\\.|[^'\\])+')"
    rb"|(?P<ident>[A-Za-z_][A-Za-z0-9_]*)"
    rb"|(?P<num>(?:0[xX][0-9a-fA-F]+|\d+\.?\d*(?:[eE][+-]?\d+)?)[uUlLfF]*)"
    rb"|(?P<punct>\.\.\.|->|\+\+|--|<<=|>>=|<<|>>|<=|>=|==|!=|&&|\|\||[-+*/%&|^!~<>=?:;,.()\[\]{}#])",
    re.S,
)

KEYWORDS = {
    "static", "extern", "register", "const", "volatile", "signed", "unsigned",
    "short", "long", "int", "char", "float", "double", "void", "struct",
    "union", "enum", "_Bool", "restrict", "inline", "__thread", "typedef",
    "auto", "__restrict", "__inline", "__inline__", "__volatile__", "__const",
}


@dataclass
class Tok:
    kind: str
    text: str
    start: int
    end: int


def tokenize(data: bytes, start: int, end: int):
    toks = []
    pos = start
    while pos < end:
        m = _TOK.match(data, pos)
        if not m:
            raise ValueError(f"cannot tokenize at offset {pos}: {data[pos:pos+20]!r}")
        kind = m.lastgroup
        pos = m.end()
        if kind in ("ws", "lcomment", "bcomment"):
            continue
        toks.append(Tok(kind, m.group().decode("utf-8", "surrogateescape"), m.start(), m.end()))
    return toks


def find_matching(toks, i, open_, close_):
    """toks[i] is an open_ token; return index of the matching close_."""
    depth = 0
    for j in range(i, len(toks)):
        t = toks[j].text
        if t == open_:
            depth += 1
        elif t == close_:
            depth -= 1
            if depth == 0:
                return j
    raise ValueError("unbalanced brackets")
