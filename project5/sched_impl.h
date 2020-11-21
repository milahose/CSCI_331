#ifndef	__SCHED_IMPL__H__
#define	__SCHED_IMPL__H__

#include "list.h"
#include "semaphore.h"

struct thread_info {
	sched_queue_t* sched_queue;
	list_elem_t* element;
	sem_t cpu;
};

struct sched_queue {
	int size;
	list_t *queue;
	list_elem_t* curr;
	sem_t cpu_lock, size_lock;
	pthread_mutex_t	queue_lock;
};

#endif /* __SCHED_IMPL__H__ */
