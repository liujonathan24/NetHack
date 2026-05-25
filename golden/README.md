# Golden trajectories for the nle_state refactor

Canonical seeded trajectories of the current (dlopen-based, pre-refactor)
NetHack env. The refactored env must produce byte-identical hash streams
for the same inputs.

## Multi-seed corpus (`golden_seed01_1k.bin` ... `golden_seed16_1k.bin`)

Sixteen seeds (1..16), 1000 gameplay steps each, recorded with the
`record-multi` mode of `verify_determinism`. Captured against
`vendor/nle/src/build/libnethack.so` (NLE 0.9.1).

- seed: NN (1..16)
- action_seed: 99
- requested n_steps: 1000 (actual recorded count is stored in the header)
- obs_size: 1767 (`NETHACK_USE_BLSTATS=1` + default chars)
- format version: 2 (gameplay-only steps; menu/--More--/yn prompts are
  drained between recorded steps so each record corresponds to a real
  game-state-advancing move, not a prompt acknowledgment)

Replay everything:

```
./ocean/nethack/verify_determinism_all.sh
```

or directly:

```
./verify_determinism replay-all --in-dir ocean/nethack/golden
```

### Truncated seeds (3, 7, 16)

Three of the sixteen seeds trigger an internal libnethack abort
(`free(): invalid pointer`) at some step late in the run. This is an
upstream NetHack bug we can't fix from the harness side. The recorder
installs a `SIGABRT` shield: on abort, it patches the file header with
the actual count flushed to disk and exits cleanly. The truncated files
are still valid, deterministic recordings — they exercise determinism
up to the point of the abort.

| seed | recorded steps |
| ---- | -------------- |
| 3    | 936            |
| 7    | 640            |
| 16   | 380            |

All other seeds record the full 1000 steps.

## Legacy: `golden_seed42_1k.bin`

The original single-seed golden (format version 1, seed=42, n_steps=1000)
has been removed in favor of the multi-seed corpus above. Format v1 files
are still readable by the harness for backward compatibility.

## Why ~1000 steps?

The current env is bit-deterministic for the first ~1480 steps and then
diverges across runs even with identical NLE seeds. Root cause:
NetHack sets `hackpid = getpid()` and `urealtime.start_timing = getnow()`
from system sources that are not covered by `nle_seeds_init_t`. The
divergence at step 1480 likely corresponds to a game event that touches
one of these (`paniclog` flush, message timestamp, time-gated check).

Sources (vendored at `vendor/nle/src/`):

- `sys/unix/unixmain.c:96` — `hackpid = getpid();`
- `src/allmain.c:645` — `urealtime.start_timing = getnow();`
- `src/end.c:1244`, `src/save.c:295`, `src/restore.c:614` — endgame `urealtime` updates
- `src/files.c:3661` — `paniclog` timestamps
- `src/hacklib.c:917` — `getnow()` wrapper around `time()`

1000 steps is below the divergence point and is enough to exercise
movement, monster spawning, room transitions, message generation, and
inventory operations — sufficient as an oracle for the RNG / `flags` /
`level` refactor subsystems.

## Future: extending the golden trajectory

To go beyond 1000 steps, patch `unixmain.c` to take `hackpid` from a
fixed sentinel (e.g. 1234) instead of `getpid()`, and add a NLE-level
`now_override` that `getnow()` consults. Once those patches land, we can
record `golden_seed42_100k.bin` and verify multi-episode trajectories.

For each refactor subsystem (RNG, flags, level, ...) re-record the
golden after the refactor lands and confirm the new file is bit-identical
to the pre-refactor one. If they differ, the refactor introduced a
behavioral change — bisect to find which subsystem.
