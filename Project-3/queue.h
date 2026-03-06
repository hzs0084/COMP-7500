#ifndef QUEUE_H
#define QUEUE_H

#include <stddef.h>
#include "job.h"

typedef enum {
  POLICY_FCFS = 0,
  POLICY_SJF = 1,
  POLICY_PRIORITY = 2
} sched_policy_t;

typedef struct {
  job_t *buf;
  size_t cap;
  size_t len;

  // Non-preemptive: keep running job at head, can track it here too.
  int next_job_id;
  sched_policy_t policy;
} job_queue_t;

int  queue_init(job_queue_t *q, size_t cap);
void queue_destroy(job_queue_t *q);

int  queue_is_empty(const job_queue_t *q);
int  queue_is_full(const job_queue_t *q);

size_t queue_len(const job_queue_t *q);

// Inserts based on current policy (FCFS append, SJF sort by expected_cpu_time, PRIORITY sort by priority then arrival)
int  queue_insert(job_queue_t *q, const job_t *job);

// Pop next job (removes from queue)
int  queue_pop(job_queue_t *q, job_t *out);

// Re-sort waiting jobs according to new policy.
// Note: This should NOT preempt a running job. In this skeleton remove jobs when dispatched,
// so simply resort the entire queue.
void queue_reschedule(job_queue_t *q, sched_policy_t new_policy);

// For list output
const char* policy_str(sched_policy_t p);

#endif