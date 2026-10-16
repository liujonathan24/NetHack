# NLE (NetHack Learning Environment) — per-environment engine

Fork of [NLE 0.9.0](https://github.com/facebookresearch/nle) / NetHack 3.6.6
that turns the game into a library holding **thousands of independent
environments in one process**, with O(1) in-memory snapshots, difficulty
knobs, level/player blobs and a secure state-mutation API. It is the engine
behind [NetHack-engine](https://github.com/liujonathan24/NetHack-engine).

## How the state is organised

Stock NetHack keeps its game state in hundreds of file-scope globals and
function-local statics, which limits it to one game per process. Here every
one of them lives in a single generated struct:

```
struct nh_globals   src/include/nh_globals.h   (generated, ~180 KB, ~670 fields)
```

reached through one thread-local pointer, `nh_g`. The rewrite is **mechanical**:
`tools/collect_globals` (libclang) takes the unmodified upstream tree, finds every
static-storage object, and

* keeps the original identifiers working through accessor macros
  (`#define moves (nh_g->moves)`), or rewrites the references to `NH_G(name)`
  where the name collides with a struct field (`level`, `flags`, `objects`, ...);
* leaves every static initializer in place as a `const` template that
  `nh_globals_init()` copies into a fresh context, so a new game starts from
  exactly the compiler's `.data` image;
* hoists the private types it needs into the header.

Nothing about the game logic changes; the rewritten library replays the same
seeds and actions to byte-identical observations. `tools/collect_globals/check_writable.sh`
proves the property after every build: the object files contain no writable
symbol outside a short, justified whitelist (function-local statics show up
there too, so a missed object cannot hide).

On top of that generated layer sits the NLE layer (`src/src/nle.c`,
`win/rl/winrl.cc`, `src/src/alloc.c`, `src/src/nle_fast_reset.c`):

* `nle_ctx_t` = one environment: coroutine, terminal emulator, settings,
  seeds, knobs, its own bump **arena** for every heap allocation the game
  makes, the rl window-port instance, and its `struct nh_globals` in the same
  contiguous block. `nle_anchor()` selects the environment on the calling
  thread, so any pool thread can step any environment.
* A **snapshot** (`nle_fr_snapshot`) copies the context block, the coroutine
  stack, the arena, the display mirror, the terminal screen and the
  off-current level files; a restore is the matching memcpys. Because every
  global is in the block, the RNG streams and all other state travel with it
  (`tests/test_interleave.c` checks solo == interleaved == restored replay).

## Build

```bash
cmake -S src -B src/build && cmake --build src/build --target nethack -j8
tests/run_tests.sh          # sentinel, multi-env, no-hang, isolation, section check
```

Produces `src/build/libnethack.so` and `src/build/dat/` (game data). The C API
(`src/include/nle.h`) is `nle_start`/`nle_step`/`nle_end` plus seeds, knobs
(`nle_tune_*`), snapshots (`nle_fr_*`), level/player blobs, `nle_set_state`,
and the curriculum helpers (`nle_goto_abs`, `nle_seat_on_stair`, ...).

## Re-running the refactor on a new upstream

```bash
pip install libclang clang
tools/collect_globals/run.sh        # on an upstream tree; see tools/collect_globals/README.md
```

## License

NetHack is distributed under the [NetHack General Public License](https://nethack.org/common/license.html).
NLE additions are under [NLE's license](https://github.com/facebookresearch/nle/blob/main/LICENSE).
