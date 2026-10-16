# collect_globals — mechanical per-environment globals refactor

NetHack keeps its game state in hundreds of file-scope globals and
function-local statics. `collect_globals` rewrites an **unmodified upstream
tree** so that every one of them lives in a single per-environment struct,
`struct nh_globals` (`src/include/nh_globals.h`), reached through one
thread-local pointer `nh_g`. Nothing about the game changes: the rewritten
library replays the same seeds and actions to byte-identical observations.

The tool is driven by libclang, so it works from the real AST, not from text
patterns: it finds every definition with static storage duration, every
reference to it (including references made inside macro bodies), and every
identifier that would collide with an accessor macro.

## What it does to each object

| object | treatment |
|---|---|
| global / file static, name free of collisions | definition removed; `#define name (nh_g->field)` (in `nh_globals.h` for globals, at the definition site for statics) |
| global / file static whose name is also a struct field, parameter, tag, ... (`level`, `flags`, `objects`, `killer`, ...) | definition removed; **every reference** rewritten to `NH_G(field)`, macro bodies included |
| function-local static | definition removed; references rewritten to `NH_G(l_file_func_name)` |
| initializer (`= {...}`) | kept in place as `const nh_tmpl_field = ...`; `nh_globals_init()` memcpys it into the struct. Initializers that take a per-env address (`&uarm`, `&HStealth`) are evaluated at init time instead |
| `static const` table whose initializer takes a per-env address | can no longer be constant: becomes per-env too |
| type defined privately in a `.c` | hoisted verbatim into `nh_globals.h`; if that is impossible (its definition names another object, or needs a header that cannot be included globally) the struct holds aligned raw bytes and the accessor casts |
| `static` the reference compiler already proved read-only | left alone (it is in `.rodata` there) |
| objects on `keep.txt` | left alone (documented process-level state) |

Objects needed by the build-time utilities that run **before** the
generated headers exist (`makedefs`, `tilemap`) keep a plain-global spelling
under `-DNH_GLOBALS_NONE`; `lev_comp` links the generated source with
`-DNH_GLOBALS_STANDALONE` and gets one context from a constructor.

## Running it

```
pip install libclang clang          # libclang python bindings
tools/collect_globals/run.sh        # on an upstream tree; see run.sh
tools/collect_globals/check_writable.sh src/build/CMakeFiles/nethack.dir
```

`check_writable.sh` is the proof: it lists every symbol in a writable
section of the built objects and fails on any that is not on
`whitelist.txt`. Function-local statics show up there too (`buf.3`), so a
missed object cannot hide. Run it after any change to the game.

`python -m nhglobals analyze --src src --build <build> --inventory
<build>/CMakeFiles/nethack.dir ...` prints the plan without editing
(counts, collisions, hoisted types, cross-check against the object files).

## Layout of the result

* `src/include/nh_globals.h` — hoisted private types, `struct nh_globals`
  (one commented field per original object, grouped by file), the `nh_g`
  declaration, `nh_globals_init()` / `nh_globals_size()`, and the accessor
  macros.
* `src/src/nh_globals.c` — `nh_g`, the init function (zero + template
  copies), and the standalone constructor for the utilities.
* `include/config.h` — `NH_G(x)` (`(nh_g->x)`, or `x` under `NH_GLOBALS_NONE`).
* `include/hack.h` — includes `nh_globals.h` last.

The NLE layer (`src/nle.c`, `win/rl/winrl.cc`) is deliberately excluded:
it owns one `struct nh_globals` per environment, calls `nh_globals_init()`
when a game starts and sets `nh_g` on every entry.

## Files

* `nhglobals/analyze.py` — AST pass (definitions, references, macro uses,
  collisions, private types, initializer dependencies).
* `nhglobals/rewrite.py` — the edits and the generated files.
* `nhglobals/cli.py` — `analyze`, `apply`, `inventory`.
* `keep.txt` — objects intentionally left process-global.
* `whitelist.txt` — what `check_writable.sh` accepts.
