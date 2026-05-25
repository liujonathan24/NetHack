# NLE (NetHack Learning Environment) — Per-Env Refactor

Modified fork of [NLE 0.9.0](https://github.com/facebookresearch/nle) / NetHack 3.6.6 for use with [PufferLib](https://github.com/PufferAI/PufferLib)'s native C vecenv.

## What changed

Stock NLE uses process-global state (NEARDATA globals, static locals, shared DLB file descriptors), which limits it to **one env per process**. Multi-env training requires either subprocess isolation or dlopen-per-copy — both slow and memory-heavy.

This fork migrates all mutable state into a per-env `nle_ctx_t` struct (~75KB), enabling thousands of independent NetHack instances in a single process with zero mutexes:

- **Global-to-local migration**: All NEARDATA/static globals → `nle_ctx_t` fields accessed via `current_nle_ctx` pointer (initial-exec TLS)
- **Function-local statics**: Per-file state structs for 15+ files with persistent locals (RNG, trap handling, display, save/restore)
- **Thread-safe DLB**: `pread()` replaces `lseek()+read()` for concurrent data-file access
- **Per-env arena allocator**: Bump allocator for level data, eliminating malloc contention
- **Direct linkage**: Single shared libnethack.so, no dlopen overhead

The C API (`nle_start`, `nle_step`, `nle_end`) is unchanged. Callers set `current_nle_ctx` before each call to select which env instance to operate on.

## Build

```bash
make -C src/build nethack -j8
```

Produces `src/build/libnethack.so` and `src/build/dat/` (game data files).

## Usage with PufferLib

This repo is cloned automatically by PufferLib's `build.sh` when building the `nethack` environment. You don't need to clone it manually.

```bash
# In PufferLib:
bash build.sh nethack    # clones this repo into vendor/nle/, builds libnethack.so
```

## Performance

With PufferLib's OMP-parallel vecenv:

| Envs | Threads | Training SPS |
|------|---------|-------------|
| 64   | 1       | 9,200       |
| 4096 | 1       | 31,300      |
| 4096 | 4       | 136,300     |

## License

NetHack is distributed under the [NetHack General Public License](https://nethack.org/common/license.html).
NLE additions are under [NLE's license](https://github.com/facebookresearch/nle/blob/main/LICENSE).
