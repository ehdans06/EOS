/********************************************************
 * Filename: core/comm.c
 *
 * Author: jtlim, RTOSLab. SNU.
 *
 * Description: message queue management.
 ********************************************************/
 /*
#include <core/eos.h>

void eos_init_mqueue(eos_mqueue_t *mq, void *queue_start, int16u_t queue_size, int8u_t msg_size, int8u_t queue_type) {

}

int8u_t eos_send_message(eos_mqueue_t *mq, void *message, int32s_t timeout) {
}

int8u_t eos_receive_message(eos_mqueue_t *mq, void *message, int32s_t timeout) {
}*/

#include <core/eos.h>

void eos_init_mqueue(eos_mqueue_t *mq, void *queue_start, int16u_t queue_size, int8u_t msg_size, int8u_t queue_type)
{
	mq->queue_size = queue_size;
	mq->msg_size = msg_size;
	mq->queue_start = queue_start;
	mq->front = queue_start;
	mq->rear = (int8u_t *)queue_start - 1;
	mq->queue_type = queue_type;
	eos_init_semaphore(&mq->putsem, queue_size, queue_type);
	eos_init_semaphore(&mq->getsem, 0, queue_type);
}

int8u_t eos_send_message(eos_mqueue_t *mq, void *message, int32s_t timeout)
{
	// wait
	if (!eos_acquire_semaphore(&mq->putsem, timeout)) return;

	// aliasing
	int8u_t *rear = (int8u_t *)mq->rear;
	int8u_t *msg = (int8u_t *)message;

	// copy
	for (int i = 0; i < mq->msg_size; ++i)
	{
		if (++rear >= (int8u_t *)mq->queue_start + mq->queue_size * mq->msg_size)
			rear = mq->queue_start;
		*rear = msg[i];
	}
	mq->rear = rear;

	// signal
	eos_release_semaphore(&mq->getsem);
}

int8u_t eos_receive_message(eos_mqueue_t *mq, void *message, int32s_t timeout)
{
	// wait
	if (!eos_acquire_semaphore(&mq->getsem, timeout)) return;

	// aliasing
	int8u_t *front = (int8u_t *)mq->front;
	int8u_t *msg = (int8u_t *)message;

	// copy
	for (int i = 0; i < mq->msg_size; ++i)
	{
		msg[i] = *front;
		if (++front >= (int8u_t *)mq->queue_start + mq->queue_size * mq->msg_size)
			front = mq->queue_start;
	}
	mq->front = front;

	// signal
	eos_release_semaphore(&mq->putsem);
}
