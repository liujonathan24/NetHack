/* nle_arena_cpp.cc
 *
 * Override C++ operator new / operator delete so that ALL heap allocations
 * made by libnethack.so (including the C++ winrl backend which uses
 * std::vector<std::unique_ptr<...>>) live in the NLE arena. This is required
 * for fast-reset snapshotting to capture the complete heap state.
 *
 * Combined with the C side (alloc.c's arena alloc() + nle_arena_free), this
 * means *every* heap pointer reachable from libnethack's writable LOAD
 * segments points into the arena, so snapshot+restore is consistent.
 *
 * Linked into libnethack.so with -Wl,-Bsymbolic-functions, intra-DSO calls
 * to operator new/delete bind here instead of libstdc++.
 */

#include <new>
#include <cstdlib>
#include <cstdio>
#include <cstddef>

#pragma GCC visibility push(hidden)

extern "C" {
    extern char  *nle_arena_base;
    extern size_t nle_arena_used;
    extern size_t nle_arena_cap;
    void nle_arena_free(void *);
    /* Implemented in alloc.c. We avoid calling it from C++ because alloc.c
     * also exposes nle_arena_free which is what we want for delete. For
     * new() we replicate the bump logic directly to avoid the dependency
     * order issue. */
    long *alloc(unsigned int);
}

static inline void *arena_alloc_cpp(std::size_t sz)
{
    /* Use the C alloc() — it bumps the same arena and panics on OOM. The
     * unsigned-int parameter is wide enough for any reasonable C++ alloc. */
    if (sz == 0) sz = 1;
    void *p = (void *) alloc((unsigned int) sz);
    return p;
}

/* Hidden visibility is CRITICAL: without it the dynamic loader interposes
 * these overrides onto the entire process, so torch / pybind11 / Python's
 * own C++ allocations all funnel through libnethack's bump arena. Then at
 * Python finalize-time, free() / delete on those pointers takes them to
 * libc which doesn't recognize them → 'free(): invalid pointer' abort. */
#define NLE_ALLOC_HIDDEN __attribute__((visibility("hidden")))

/* Throwing forms */
NLE_ALLOC_HIDDEN void *operator new(std::size_t sz)            { return arena_alloc_cpp(sz); }
NLE_ALLOC_HIDDEN void *operator new[](std::size_t sz)          { return arena_alloc_cpp(sz); }

/* nothrow forms */
NLE_ALLOC_HIDDEN void *operator new(std::size_t sz, const std::nothrow_t &) noexcept
{ return arena_alloc_cpp(sz); }
NLE_ALLOC_HIDDEN void *operator new[](std::size_t sz, const std::nothrow_t &) noexcept
{ return arena_alloc_cpp(sz); }

static inline void dbg_del(void *p, const char *tag) {
    (void) tag;
    nle_arena_free(p);
}
NLE_ALLOC_HIDDEN void operator delete(void *p) noexcept                 { dbg_del(p,"d"); }
NLE_ALLOC_HIDDEN void operator delete[](void *p) noexcept               { dbg_del(p,"da"); }
NLE_ALLOC_HIDDEN void operator delete(void *p, std::size_t) noexcept    { dbg_del(p,"ds"); }
NLE_ALLOC_HIDDEN void operator delete[](void *p, std::size_t) noexcept  { dbg_del(p,"das"); }
NLE_ALLOC_HIDDEN void operator delete(void *p, const std::nothrow_t &) noexcept    { dbg_del(p,"dnt"); }
NLE_ALLOC_HIDDEN void operator delete[](void *p, const std::nothrow_t &) noexcept  { dbg_del(p,"dant"); }

#pragma GCC visibility pop
