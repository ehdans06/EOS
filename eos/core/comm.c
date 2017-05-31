/********************************************************
 * Filename: core/comm.c
 *
 * Author: jtlim, RTOSLab. SNU.
 *
 * Description: message queue management.
 ********************************************************/

#include <core/eos.h>

void eos_init_mqueue(eos_mqueue_t *mq, void *queue_start, int16u_t queue_size, int8u_t msg_size, int8u_t queue_type) {
  mq->queue_size = queue_size;
  mq->msg_size = msg_size;

  // front always have its value, and rear always does not have its value
  // So, if the addresses of front & rear are equal, it means, the queue is empty
  mq->queue_start = queue_start;
  mq->front = queue_start;
  mq->rear = queue_start;

  mq->queue_type = queue_type;
  eos_init_semaphore(&(mq->putsem), queue_size, queue_type);
  eos_init_semaphore(&(mq->getsem), 0, queue_type);
}

int8u_t eos_send_message(eos_mqueue_t *mq, void *message, int32s_t timeout) {

  // get semaphore
  if (!eos_acquire_semaphore((&mq->putsem), timeout))  return;

  int8u_t* msge = (int8u_t*)message;

  for (int i = 0; i < mq->msg_size; i++) {
    // send the message in the rear part of queue
    *(mq->rear) = msge[i];

    // if 'rear' points the end of the queue, make 'rear' point the start of the queue
    // if not, make 'rear' point the next entry of the queue
    if (mq->rear == (int8u_t*)mq->queue_start + mq->queue_size * mq->msg_size){
      mq->rear = mq->queue_start;
    }
    else {
      mq->rear++;
    }
  }
  // release semaphore
  eos_release_semaphore(&mq->getsem);
}

int8u_t eos_receive_message(eos_mqueue_t *mq, void *message, int32s_t timeout) {

  // get semaphore
  if (!eos_acquire_semaphore(&(mq->getsem), timeout)) return;

  int8u_t* msge = (int8u_t*)message;

  for (int i =0; i < mq->msg_size; i++) {
    // get the message from the front part of queue
    msge[i] = *((int8u_t*)(mq->front));

    // if 'front' points the end of the queue, make 'front' point the start of the queue
    // if not, make 'rear' point the next entry of the queue
    if (mq->front == (int8u_t*)mq->queue_start + mq->queue_size * mq->msg_size){
      mq->front = mq->queue_start;
    }
    else {
      mq->front++;
    }
  }
  // release semaphore
  eos_release_semaphore(&mq->putsem);
}
