
#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "checkout.h"
#include "config.h"
#include "../helper/logging.h"

checkout_t *init_checkout() {
  checkout_t *c = malloc(sizeof(checkout_t));
  if (c == NULL) {
    log_error("[%s] (%s) Failed to allocate space for the pool\n",
              __FILE_NAME__, __func__);
    return NULL;
  }

  if (pthread_mutex_init(&(c->mx), NULL) != 0) {
    log_error("[%s] (%s) Could not initializate the mutex! Cause: %s\n", 
              __FILE_NAME__, __func__, strerror(errno));
    perror("pthread_mutex_init: ");
		free(c);
    return NULL;
  }

  // get random time
  c->service_time = rand() % (MAX_TIME + 1 - MIN_TIME) + MIN_TIME;

  return c;
}

void destroy_checkout(checkout_t *c) {
  if (c != NULL) {
    pthread_mutex_destroy(&(c->mx));
    free(c);
  }
}

// lock the mutex and get the service time of checkout
int enter_checkout(checkout_t *c) {
  if (c == NULL)
    return -1;
  
  pthread_mutex_lock(&(c->mx));
  return c->service_time;
}

// release the mutex
void leave_checkout(checkout_t *c) {
  if (c != NULL) {
    pthread_mutex_unlock(&(c->mx));
  }
}