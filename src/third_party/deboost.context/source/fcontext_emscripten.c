/*
 * fcontext_emscripten.c — emscripten_fiber-backed implementation of the
 * deboost.context fcontext API, for WebAssembly builds.
 *
 * boost.context / deboost's fcontext uses per-architecture hand-written
 * assembly for stack switching, which has no WebAssembly backend. Emscripten's
 * fiber API (emscripten/fiber.h, requires -sASYNCIFY) provides an equivalent
 * switchable-stack primitive in WASM linear memory. This file re-implements
 * make_fcontext / jump_fcontext / create_fcontext_stack / destroy_fcontext_stack
 * on top of it so NLE's coroutine step model (nle_start / nle_step / nle_yield
 * in src/nle.c) works unchanged in the browser.
 *
 * Semantics reproduced (standard fcontext contract):
 *   jump_fcontext(to, vp) transfers control to `to`, which observes a transfer
 *   {ctx = the context we came from, data = vp}. When some later jump_fcontext
 *   targets us, our own jump_fcontext call returns that transfer.
 */
#include "fcontext/fcontext.h"

#include <emscripten/fiber.h>
#include <stdlib.h>

/* Asyncify stack per fiber: holds unwound C call frames across a swap. NetHack
 * yields from deep inside the game loop (win/rl port -> nle_yield), so keep it
 * generous to avoid asyncify-stack overflow. */
#define NLE_ASYNCIFY_STACK_SIZE (1u << 20) /* 1 MiB */

typedef struct fib_s {
    emscripten_fiber_t  fiber;
    pfn_fcontext        entry;    /* coroutine body (make_fcontext fibers only) */
    void               *asyncify; /* owned asyncify stack buffer */
    fcontext_transfer_t incoming; /* transfer observed when this fiber resumes */
} fib_t;

/* Single-env-per-module: these are effectively globals (no pthreads in the
 * browser build), but keep _Thread_local so a pthreads build stays correct. */
static _Thread_local fib_t  g_main;
static _Thread_local int    g_main_ready = 0;
static _Thread_local fib_t *g_current = 0;

static void ensure_main(void)
{
    if (!g_main_ready) {
        g_main.asyncify = malloc(NLE_ASYNCIFY_STACK_SIZE);
        emscripten_fiber_init_from_current_context(
            &g_main.fiber, g_main.asyncify, NLE_ASYNCIFY_STACK_SIZE);
        g_current = &g_main;
        g_main_ready = 1;
    }
}

static void trampoline(void *arg)
{
    fib_t *self = (fib_t *)arg;
    self->entry(self->incoming); /* fcontext entry receives the initial transfer */
    abort();                     /* fcontext coroutines are not supposed to return */
}

fcontext_t make_fcontext(void *sp, size_t size, pfn_fcontext corofn)
{
    ensure_main();
    fib_t *f = (fib_t *)calloc(1, sizeof(fib_t));
    f->entry = corofn;
    f->asyncify = malloc(NLE_ASYNCIFY_STACK_SIZE);
    /* We own both ends of the stack convention here (our create_fcontext_stack
     * returns the malloc base), so pass the base + size straight through;
     * emscripten_fiber_init derives base/limit internally. */
    emscripten_fiber_init(&f->fiber, trampoline, f, sp, size,
                          f->asyncify, NLE_ASYNCIFY_STACK_SIZE);
    return (fcontext_t)f;
}

fcontext_transfer_t jump_fcontext(fcontext_t const to, void *vp)
{
    ensure_main();
    fib_t *from = g_current;
    fib_t *dst = (fib_t *)to;
    dst->incoming.ctx = (fcontext_t)from;
    dst->incoming.data = vp;
    g_current = dst;
    emscripten_fiber_swap(&from->fiber, &dst->fiber);
    /* Resumed: `from->incoming` was set by whoever swapped back into us. */
    return from->incoming;
}

fcontext_transfer_t ontop_fcontext(fcontext_t const to, void *vp,
                                   fcontext_transfer_t (*fn)(fcontext_transfer_t))
{
    (void)to;
    (void)vp;
    (void)fn;
    abort(); /* not used by NLE */
}

/* Give back what make_fcontext allocated. The asm backends place the context
 * inside the caller's stack and own nothing, so the API historically had no
 * destroy call; here each context is a heap fib_t plus a 1 MiB asyncify stack,
 * so without this every nle_start leaks both. Never frees the main-thread
 * context, which is a static owned by ensure_main(). */
void destroy_fcontext(fcontext_t ctx)
{
    fib_t *f = (fib_t *) ctx;
    if (!f || f == &g_main)
        return;
    free(f->asyncify);
    free(f);
}

fcontext_stack_t create_fcontext_stack(size_t size)
{
    fcontext_stack_t s;
    if (size == 0)
        size = 64 * 1024;
    s.sptr = malloc(size);
    s.ssize = s.sptr ? size : 0;
    return s;
}

void destroy_fcontext_stack(fcontext_stack_t *s)
{
    if (s && s->sptr) {
        free(s->sptr);
        s->sptr = 0;
        s->ssize = 0;
    }
}
