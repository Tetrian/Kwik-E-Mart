#ifndef TS_QUEUE_H
#define TS_QUEUE_H

#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>

/* ************************************************************************** */

typedef struct node_t {
  void *value;
  struct node_t *next;
} node_t;

// thread safe queue
typedef struct ts_queue_t {
  pthread_mutex_t mx;
  pthread_cond_t cv;

  node_t *head;
  node_t *tail;

  size_t size;
  bool keep;
} ts_queue_t;

/* ************************************************************************** */

ts_queue_t *init_queue();

void destroy_queue(ts_queue_t *);

void enqueue(ts_queue_t *, void *);

void *dequeue(ts_queue_t *);

// wake all operation in enqueue
void wake_all(ts_queue_t *);

/* ************************************************************************** */

#endif
