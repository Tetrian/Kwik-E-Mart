
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "ts_queue.h"
#include "logging.h"
#include "../core/config.h"

ts_queue_t *init_queue() {
  ts_queue_t *tsq = malloc(sizeof(ts_queue_t));
  if (tsq == NULL) {
    log_error("[%s] (%s) Failed to allocate space for the queue\n",
              __FILE_NAME__, __func__);
    return NULL;
  }

  if (pthread_mutex_init(&(tsq->mx), NULL) != 0) {
    log_error("[%s] (%s) Could not initializate the mutex! Cause: %s\n", 
              __FILE_NAME__, __func__, strerror(errno));
    perror("pthread_mutex_init: ");
		free(tsq);
    return NULL;
  }

  if (pthread_cond_init(&(tsq->cv), NULL) != 0) {
    log_error("[%s] (%s) Could not initializate the condition variable! "
              "Cause: %s\n", __FILE_NAME__, __func__, strerror(errno));
    perror("pthread_cond_init: ");
    pthread_mutex_destroy(&(tsq->mx));
		free(tsq);
    return NULL;
  }

  tsq->head = tsq->tail = NULL;
  tsq->keep = true;
  tsq->size = 0;

  return tsq;
}

void destroy_queue(ts_queue_t *tsq) {
  if (tsq != NULL) {
    while (tsq->head != NULL)
      dequeue(tsq);
    
    pthread_mutex_destroy(&(tsq->mx));
    pthread_cond_destroy(&(tsq->cv));
    
    free(tsq);
  }
}

void enqueue(ts_queue_t *tsq, void *val) {
  node_t *node = malloc(sizeof(node_t));
  if (node == NULL) {
    log_error("[%s] (%s) Failed to allocate space for the node n*%d\n",
              __FILE_NAME__, __func__, tsq->size);
    return;
  }
  node->value = val;

  pthread_mutex_lock(&(tsq->mx));
  if (tsq->size == 0)
    tsq->head = tsq->tail = node;
  else {
    tsq->tail->next = node;
    tsq->tail = node;
  }
  tsq->size++;

  pthread_cond_signal(&(tsq->cv));
  pthread_mutex_unlock(&(tsq->mx));
}

void *dequeue(ts_queue_t *tsq) {
  pthread_mutex_lock(&(tsq->mx));
  
  while (tsq->size == 0 && tsq->keep)
    pthread_cond_wait(&(tsq->cv), &(tsq->mx));
  if (tsq->size == 0) {
    pthread_mutex_unlock(&(tsq->mx));
    return NULL;
  }

  void *val = tsq->head->value;

  if (tsq->size == 1) {
    free(tsq->head);
    tsq->head = tsq->tail = NULL;
  } else {
    node_t *node = tsq->head;
    tsq->head = (tsq->head)->next;
    free(node);
  }
  tsq->size--;

  pthread_mutex_unlock(&(tsq->mx));

  return val;
}

void* peek(ts_queue_t *tsq) {
  pthread_mutex_lock(&(tsq->mx));
  void *val = tsq->head->value;
  pthread_mutex_unlock(&(tsq->mx));
  return val;
}

size_t get_size(ts_queue_t *tsq) {
  pthread_mutex_lock(&(tsq->mx));
  size_t size = tsq->size;
  pthread_mutex_unlock(&(tsq->mx));
  return size;
}

void wake_all(ts_queue_t *tsq) {
  pthread_mutex_lock(&(tsq->mx));
  tsq->keep = false;
  pthread_cond_broadcast(&(tsq->cv));
  pthread_mutex_unlock(&(tsq->mx));
}

bool is_keep(ts_queue_t *tsq) {
  pthread_mutex_lock(&(tsq->mx));
  bool value = tsq->keep;
  pthread_mutex_unlock(&(tsq->mx));
  return value;
}