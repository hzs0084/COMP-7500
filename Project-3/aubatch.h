// aubatch.h
#ifndef AUBATCH_H
#define AUBATCH_H

#include <pthread.h>
#include "queue.h"
#include "job.h"

typedef struct {

  // Metrics
  long total_jobs_submitted;
  long total_jobs_completed;

  double sum_turnaround; // (end - arrival)
  double sum_cpu;        // treat as wall time for now
  double sum_waiting;    // turnaround - cpu

  time_t first_start;
  time_t last_end;
  int has_first_start;
  job_queue_t queue;

  // Track the currently running job (non-preemptive behavior)
  int has_running;
  job_t running_job;

  pthread_mutex_t mtx;
  pthread_cond_t  cv_not_empty;
  pthread_cond_t  cv_not_full;

  int shutting_down;
} aubatch_state_t;

#endif