/********************************************************
 * Filename: core/task.c
 *
 * Author: parkjy, RTOSLab. SNU.
 *
 * Description: task management.
 ********************************************************/
#include <core/eos.h>

#define READY		1
#define RUNNING		2
#define WAITING		3

/*
 * Queue (list) of tasks that are ready to run.
 */
static _os_node_t *_os_ready_queue[LOWEST_PRIORITY + 1];

/*
 * Pointer to TCB of running task
 */
static eos_tcb_t *_os_current_task;

int32u_t eos_create_task(eos_tcb_t *task, addr_t sblock_start, size_t sblock_size, void (*entry)(void *arg), void *arg, int32u_t priority) {

	// set values on TCB
	task->state = READY;
	task->priority = priority;
	task->period = 0;
	task->sp = _os_create_context(sblock_start, sblock_size, entry, arg);

	task->node = (_os_node_t*)malloc(sizeof(_os_node_t));
	task->node->ptr_data = task;
	task->node->priority = priority;

	task->alarm = (eos_alarm_t*)malloc(sizeof(eos_alarm_t));
	task->alarm->timeout = 0;
	task->alarm->alarm_queue_node.ptr_data = task;
	task->alarm->alarm_queue_node.previous = NULL;
	task->alarm->alarm_queue_node.next = NULL;
	task->alarm->alarm_queue_node.priority = 0;

	// push the task in ready queue
	_os_add_node_tail(&_os_ready_queue[priority], task->node);
	_os_set_ready(priority);
	PRINT("task: 0x%x, priority: %d\n", (int32u_t)task, priority);

	return 1;
}

int32u_t eos_destroy_task(eos_tcb_t *task) {
}

void eos_schedule() {

	// if the current task exists, save the context
	if (eos_get_current_task() != NULL) {
		eos_tcb_t* cur_task = eos_get_current_task();
		addr_t sp = _os_save_context();
		if (sp == NULL) return;
		else {
			cur_task->state = READY;
			cur_task->sp = sp;
			if (cur_task->period == 0){
				_os_add_node_tail(&_os_ready_queue[cur_task->priority], cur_task->node);
				_os_set_ready(cur_task->priority);
			}
		}
	}

	// restore context of the next task
	int32u_t high_pri = _os_get_highest_priority();
	_os_node_t* next_task = _os_ready_queue[high_pri];
	if (next_task == NULL) return;
	else {
		_os_remove_node(&_os_ready_queue[high_pri], next_task);
		if (_os_ready_queue[high_pri] == NULL) _os_unset_ready(high_pri);
		_os_current_task = next_task->ptr_data;
		_os_current_task->state = RUNNING;
		_os_restore_context(_os_current_task->sp);
	}
}

eos_tcb_t *eos_get_current_task() {
	return _os_current_task;
}

void eos_change_priority(eos_tcb_t *task, int32u_t priority) {
}

int32u_t eos_get_priority(eos_tcb_t *task) {
}

void eos_set_period(eos_tcb_t *task, int32u_t period){
	task->period = period;
}

int32u_t eos_get_period(eos_tcb_t *task) {
}

int32u_t eos_suspend_task(eos_tcb_t *task) {
}

int32u_t eos_resume_task(eos_tcb_t *task) {
}

void eos_sleep(int32u_t tick) {
	eos_tcb_t* cur_task = _os_current_task;
	cur_task->state = WAITING;
	int32u_t timeout = (tick == 0) ? eos_get_system_timer()->tick + cur_task->period : eos_get_system_timer()->tick + tick;
	eos_set_alarm(eos_get_system_timer(), cur_task->alarm, timeout, &_os_wakeup_sleeping_task, cur_task);
	eos_schedule();
}

void _os_init_task() {
	PRINT("initializing task module.\n");

	/* init current_task */
	_os_current_task = NULL;

	/* init multi-level ready_queue */
	int32u_t i;
	for (i = 0; i < LOWEST_PRIORITY; i++) {
		_os_ready_queue[i] = NULL;
	}
}

void _os_wait(_os_node_t **wait_queue) {
}

void _os_wakeup_single(_os_node_t **wait_queue, int32u_t queue_type) {
}

void _os_wakeup_all(_os_node_t **wait_queue, int32u_t queue_type) {
}

void _os_wakeup_sleeping_task(void *arg) {
	eos_tcb_t* wake_task = (eos_tcb_t*)arg;
	wake_task->state = READY;
	_os_set_ready(wake_task->priority);
	_os_add_node_tail(&_os_ready_queue[wake_task->priority], wake_task->node);
}
