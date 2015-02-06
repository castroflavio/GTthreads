#ifndef __GT_CONTEXT__
#define __GT_CONTEXT__
#include "gtthread.h"
#include "gt_mutex.h"
#include "gt_signals.h"
#include "gt_atomic_ops.h"
#include "gt_context.h"

extern ucontext_t scheduler;
extern ucontext_t main_context;
extern char schedule_stack[STACKSIZE];

//Context operations
extern void schedule();
extern void make_sched_context();
extern void make_main_context();
extern int make_thread_context( gtthread_t *thread, void *(*start_routine)(void *), void *arg );

//Internal operations
extern void init_thread_structs();
extern void generate_thread_id( gtthread_t *thread );

#endif
