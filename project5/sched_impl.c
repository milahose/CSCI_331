#include <stdlib.h>
#include "scheduler.h"
#include "sched_impl.h"
#include "pthread.h"
#include "semaphore.h"
#include "list.h"

/* Fill in your scheduler implementation code below: */

/* Thread operations */

/* Initialize a thread_info_t */
static void init_thread_info(thread_info_t *info, sched_queue_t *queue) {
  info->sched_queue = queue;
	sem_init(&info->cpu, 0, 0);
}
/* Release the resources associated with a thread_info_t */
static void destroy_thread_info(thread_info_t *info) {
  info->sched_queue = NULL;
	info->element = NULL;
}
/* Block until the thread can enter the scheduler queue. */
static void enter_sched_queue(thread_info_t *info) {
	sem_wait(&info->sched_queue->size_lock);
	sched_queue_t* sched_queue = info->sched_queue;
	pthread_mutex_lock(&sched_queue->queue_lock);
	list_elem_t* elem = (list_elem_t*) malloc(sizeof(list_elem_t));
	elem->datum = (void*) info;
	list_insert_tail(sched_queue->queue, elem);
	info->element = list_get_tail(sched_queue->queue);
	pthread_mutex_unlock(&sched_queue->queue_lock);
}
/* Remove the thread from the scheduler queue. */
static void leave_sched_queue(thread_info_t *info) {
  pthread_mutex_lock(&info->sched_queue->queue_lock);
	list_remove_elem(info->sched_queue->queue, info->element);
	info->sched_queue->curr = NULL;
	sem_post(&info->sched_queue->size_lock);
	pthread_mutex_unlock(&info->sched_queue->queue_lock);
}
/* While on the scheduler queue, block until thread is scheduled. */
static void wait_for_cpu(thread_info_t *info) {
  sem_wait(&info->cpu);
}
/* Voluntarily relinquish the CPU when this thread's timeslice is
    * over (cooperative multithreading). */
static void release_cpu(thread_info_t *info) {
  sem_post(&info->sched_queue->cpu_lock);
}



/* Queue/scheduler operations */

/* Initialize a sched_queue_t */
static void init_sched_queue(sched_queue_t *queue, int queue_size) {
	queue->queue = (list_t*) malloc(sizeof(list_t));
	list_init(queue->queue);
	queue->size = queue_size;
	queue->curr = NULL;
	sem_init(&queue->cpu_lock, 0, 0);
	sem_init(&queue->size_lock, 0, queue_size);
	pthread_mutex_init(&queue->queue_lock, NULL);
}
/* Release the resources associated with a sched_queue_t */
static void destroy_sched_queue(sched_queue_t *queue) {
	list_elem_t *curr;
	while( (curr = list_get_head(queue->queue)) != NULL){
		list_remove_elem(queue->queue, curr);
		free(curr);
	}
}
/* Allow a worker thread to execute. */
static void wake_up_worker(thread_info_t *info) {
	sem_post(&info->cpu);
}
/* Block until the current worker thread relinquishes the CPU. */
static void wait_for_worker(sched_queue_t *queue) {
	sem_wait(&queue->cpu_lock);
}
/* Select the next worker thread to execute.
    * Returns NULL if the scheduler queue is empty. */
static thread_info_t *next_worker_fifo(sched_queue_t *queue) {
	pthread_mutex_lock(&queue->queue_lock);
	list_elem_t* head = list_get_head(queue->queue);

	if(head == NULL){
		pthread_mutex_unlock(&queue->queue_lock);
		return NULL;
	}

	thread_info_t * next_worker_fifo = (thread_info_t *)(head->datum);
	pthread_mutex_unlock(&queue->queue_lock);
	return next_worker_fifo;
}
static thread_info_t *next_worker_rr(sched_queue_t *queue) {
	pthread_mutex_lock(&queue->queue_lock);
	if(queue->curr != NULL){
		list_remove_elem(queue->queue, queue->curr);
		list_insert_tail(queue->queue, queue->curr);
	}

	queue->curr = list_get_head(queue->queue);

	if(queue->curr == NULL){
		pthread_mutex_unlock(&queue->queue_lock);
		return NULL;
	}

	thread_info_t * next_worker_fifo = (thread_info_t *)(queue->curr->datum);
	pthread_mutex_unlock(&queue->queue_lock);
	return next_worker_fifo;
}
/* Block until at least one worker thread is in the scheduler queue. */
static void wait_for_queue(sched_queue_t *queue) {
	int complete, curr_size = 0;

	if (!complete) {
		while (curr_size == 0) {
			curr_size=list_size(queue->queue);
		}
		complete = 1;
	}
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
