#ifndef CHECKOUT_H
#define CHECKOUT_H

#include <pthread.h>
#include <sys/types.h>

#include "../helper/ts_queue.h"

/* ************************************************************************** */

#define MIN_TIME 1
#define MAX_TIME 5

/* ************************************************************************** */

typedef struct checkout_t {
  pthread_mutex_t mx;
  int service_time;
} checkout_t;

/* ************************************************************************** */

checkout_t *init_checkout();

void destroy_checkout(checkout_t *);

// lock the mutex and get the service time of checkout
int enter_checkout(checkout_t *);

// release the mutex
void leave_checkout(checkout_t *);

/* ************************************************************************** */

#endif