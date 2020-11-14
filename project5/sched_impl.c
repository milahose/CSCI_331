#include "scheduler.h"
#include "sched_impl.h"

/* Fill in your scheduler implementation code below: */

/* Thread operations */

/* Initialize a thread_info_t */
static void init_thread_info(thread_info_t *info, sched_queue_t *queue) {
    /*...Code goes here...*/
}
/* Release the resources associated with a thread_info_t */
static void destroy_thread_info(thread_info_t *info) {
    /*...Code goes here...*/
}
/* Block until the thread can enter the scheduler queue. */
static void enter_sched_queue(thread_info_t *info) {
    /*...Code goes here...*/
}
/* Remove the thread from the scheduler queue. */
static void leave_sched_queue(thread_info_t *info) {
    /*...Code goes here...*/
}
/* While on the scheduler queue, block until thread is scheduled. */
static void wait_for_cpu(thread_info_t *info) {
    /*...Code goes here...*/
}
/* Voluntarily relinquish the CPU when this thread's timeslice is
    * over (cooperative multithreading). */
static void release_cpu(thread_info_t *info) {
    /*...Code goes here...*/
}



/* Queue/scheduler operations */

/* Initialize a sched_queue_t */
static void init_sched_queue(sched_queue_t *queue, int queue_size) {
}
/* Release the resources associated with a sched_queue_t */
static void destroy_sched_queue(sched_queue_t *queue) {
}
/* Allow a worker thread to execute. */
static void wake_up_worker(thread_info_t *info) {
}
/* Block until the current worker thread relinquishes the CPU. */
static void wait_for_worker(sched_queue_t *queue) {
}
/* Select the next worker thread to execute.
    * Returns NULL if the scheduler queue is empty. */
static thread_info_t *next_worker_fifo(sched_queue_t *queue) {
}
static thread_info_t *next_worker_rr(sched_queue_t *queue) {
}
/* Block until at least one worker thread is in the scheduler queue. */
static void wait_for_queue(sched_queue_t *queue) {
}

/* You need to statically initialize these structures: */
sched_impl_t sched_fifo = {
    { init_thread_info,
      destroy_thread_info,
      enter_sched_queue,
      leave_sched_queue,
      wait_for_cpu,
      release_cpu,
    },
    { init_sched_queue,
      destroy_sched_queue,
      wake_up_worker,
      wait_for_worker,
      next_worker_fifo,
      wait_for_queue,
    } };
sched_impl_t sched_rr = {
    { init_thread_info,
      destroy_thread_info,
      enter_sched_queue,
      leave_sched_queue,
      wait_for_cpu,
      release_cpu,
    },
    { init_sched_queue,
      destroy_sched_queue,
      wake_up_worker,
      wait_for_worker,
      next_worker_rr,
      wait_for_queue,
    } };
