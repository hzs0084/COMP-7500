#ifndef AUBATCH_H
#define AUBATCH_H

#include <pthread.h>
#include "queue.h"

// Shared state
typedef struct {
  job_queue_t queue;

  pthread_mutex_t mtx;
  pthread_cond_t  cv_not_empty;
  pthread_cond_t  cv_not_full;

  int shutting_down; // set by quit
} aubatch_state_t;

#endif