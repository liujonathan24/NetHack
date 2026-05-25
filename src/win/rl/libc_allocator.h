/* libc_allocator.h
 *
 * A minimal C++ allocator that routes through libc
 * std::malloc / std::free instead of through global operator new / delete.
 *
 * Why: libnethack.so overrides global operator new / delete in
 * vendor/nle/src/src/nle_arena_cpp.cc so that they bump-allocate from the
 * shared 16 GB NLE arena. That arena is shared across all PufferLib NetHack
 * envs (cf. nethack.h ~line 65). Any STL container or std::string that uses
 * the default allocator therefore stores its heap nodes inside the arena.
 *
 * The per-env C++ shim state anchored from nle_ctx_t (s_win_proc_calls,
 * s_netHackRL_instance, and everything they own transitively) is per-env
 * logically, but its arena-resident bytes are reachable from any env and can
 * be zeroed / overwritten by another env's libnethack activity
 * (freedynamicdata, save-cleanup memsets, etc.). The deque destructor then
 * pop_back's on a zero-initialized control block and segfaults — see
 * ocean/nethack/experiments/exp_033_cluster_az/CRASH_DIAG.md.
 *
 * Using this allocator on the per-env C++ containers puts their backing
 * memory in glibc's heap, which libnethack never writes into.
 *
 * The override only intercepts operator new / delete, NOT the malloc / free
 * symbols (verified in nle_arena_cpp.cc). std::malloc / std::free are safe.
 */

#ifndef NLE_WIN_RL_LIBC_ALLOCATOR_H
#define NLE_WIN_RL_LIBC_ALLOCATOR_H

#include <cstddef>
#include <cstdlib>
#include <new>      // std::bad_alloc

namespace nethack_rl
{

template <class T>
struct LibcAllocator {
    using value_type = T;

    LibcAllocator() noexcept = default;

    template <class U>
    LibcAllocator(const LibcAllocator<U> &) noexcept
    {
    }

    T *
    allocate(std::size_t n)
    {
        if (n == 0) n = 1;
        void *p = std::malloc(n * sizeof(T));
        if (!p) throw std::bad_alloc();
        return static_cast<T *>(p);
    }

    void
    deallocate(T *p, std::size_t /*n*/) noexcept
    {
        std::free(p);
    }
};

template <class T, class U>
bool
operator==(const LibcAllocator<T> &, const LibcAllocator<U> &) noexcept
{
    return true;
}

template <class T, class U>
bool
operator!=(const LibcAllocator<T> &, const LibcAllocator<U> &) noexcept
{
    return false;
}

} // namespace nethack_rl

#endif /* NLE_WIN_RL_LIBC_ALLOCATOR_H */
