/********************************************************
 * Filename: core/sync.c
 *
 * Author: wsyoo, RTOSLab. SNU.
 *
 * Description: semaphore, condition variable management.
 ********************************************************/
#include <core/eos.h>
#define INFINITY	0x7EEEEEEE
extern int32u_t _eflags;

void eos_init_semaphore(eos_semaphore_t *sem, int32u_t initial_count, int8u_t queue_type) {
	/* initialization */
	sem->count = initial_count;
	sem->wait_queue = NULL;
	sem-> queue_type = queue_type;
}

int32u_t eos_acquire_semaphore(eos_semaphore_t *sem, int32s_t timeout) {
	eos_disable_interrupt();

	// if the resource is available, return 1
	if (sem->count > 0) {
		sem->count--;
		eos_enable_interrupt();
		return 1;
	}
	// if timeout is negative (failed), return 0;
	if (timeout < 0) {
		eos_enable_interrupt();
		return 0;
	}
	// if task should use the resource right after the resource is available,
	if (timeout == 0) {
		while (1) {
			// push current task into waiting queue, and yield a CPU
			eos_tcb_t* cur_task = eos_get_current_task();
			cur_task->state = 3; // WAITING
			if (sem->queue_type == FIFO) {
				_os_add_node_tail(&(sem->wait_queue), &(cur_task->node));
			}
			else if (sem->queue_type == PRIORITY) {
				_os_add_node_priority(&(sem->wait_queue), &(cur_task->node));
			}
			eos_enable_interrupt();
			eos_sleep(INFINITY);

			// after waking up, if the resource is available, return 1
			eos_disable_interrupt();
			if (sem->count > 0) {
				sem->count--;
				eos_enable_interrupt();
				return 1;
			}
		}
	}
	// if timeout has positive value, make current task sleep for timeout
	// and after waking up, if the resource is available, return 1
	while (1) {
		eos_enable_interrupt();
		eos_sleep(timeout);
		eos_disable_interrupt();
		if (sem->count > 0) {
			sem->count--;
			eos_enable_interrupt();
			return 1;
		}
	}
}

void eos_release_semaphore(eos_semaphore_t *sem) {
	sem->count++;
	if (sem->wait_queue) {
		eos_tcb_t* wake_task = (eos_tcb_t*)(sem->wait_queue->ptr_data);
		eos_set_alarm(eos_get_system_timer(), wake_task->alarm, 0, NULL, NULL);
		_os_remove_node(&(sem->wait_queue), sem->wait_queue);
		_os_wakeup_sleeping_task((void*)wake_task);
	}
}

void eos_init_condition(eos_condition_t *cond, int32u_t queue_type) {
	/* initialization */
	cond->wait_queue = NULL;
	cond->queue_type = queue_type;
}

void eos_wait_condition(eos_condition_t *cond, eos_semaphore_t *mutex) {
	/* release acquired semaphore */
	eos_release_semaphore(mutex);
	/* wait on condition's wait_queue */
	_os_wait(&cond->wait_queue);
	/* acquire semaphore before return */
	eos_acquire_semaphore(mutex, 0);
}

void eos_notify_condition(eos_condition_t *cond) {
	/* select a task that is waiting on this wait_queue */
	_os_wakeup_single(&cond->wait_queue, cond->queue_type);
}
