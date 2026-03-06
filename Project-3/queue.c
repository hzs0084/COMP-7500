#include "queue.h"
#include <stdlib.h>
#include <string.h>

static int cmp_fcfs(const void *a, const void *b) {
  const job_t *ja = (const job_t*)a;
  const job_t *jb = (const job_t*)b;
  if (ja->arrival_time < jb->arrival_time) return -1;
  if (ja->arrival_time > jb->arrival_time) return  1;
  // tie-breaker: id
  return (ja->id - jb->id);
}

static int cmp_sjf(const void *a, const void *b) {
  const job_t *ja = (const job_t*)a;
  const job_t *jb = (const job_t*)b;
  if (ja->expected_cpu_time != jb->expected_cpu_time)
    return (ja->expected_cpu_time - jb->expected_cpu_time);
  // tie-break by arrival then id
  return cmp_fcfs(a, b);
}

static int cmp_priority(const void *a, const void *b) {
  const job_t *ja = (const job_t*)a;
  const job_t *jb = (const job_t*)b;
  // Smaller number = higher priority (common convention).
  if (ja->priority != jb->priority)
    return (ja->priority - jb->priority);
  // tie-break by arrival then id
  return cmp_fcfs(a, b);
}

static int (*get_cmp(sched_policy_t p))(const void*, const void*) {
  switch (p) {
    case POLICY_SJF:      return cmp_sjf;
    case POLICY_PRIORITY: return cmp_priority;
    case POLICY_FCFS:
    default:              return cmp_fcfs;
  }
}

int queue_init(job_queue_t *q, size_t cap) {
  if (!q || cap == 0) return -1;
  q->buf = (job_t*)calloc(cap, sizeof(job_t));
  if (!q->buf) return -1;
  q->cap = cap;
  q->len = 0;
  q->next_job_id = 1;
  q->policy = POLICY_FCFS;
  return 0;
}

void queue_destroy(job_queue_t *q) {
  if (!q) return;
  free(q->buf);
  q->buf = NULL;
  q->cap = 0;
  q->len = 0;
}

int queue_is_empty(const job_queue_t *q) { return (!q || q->len == 0); }
int queue_is_full (const job_queue_t *q) { return (q && q->len >= q->cap); }
size_t queue_len(const job_queue_t *q) { return q ? q->len : 0; }

int queue_insert(job_queue_t *q, const job_t *job) {
  if (!q || !job) return -1;
  if (queue_is_full(q)) return -1;

  q->buf[q->len] = *job;
  q->len++;

  // Keep queue sorted per policy
  qsort(q->buf, q->len, sizeof(job_t), get_cmp(q->policy));
  return 0;
}

int queue_pop(job_queue_t *q, job_t *out) {
  if (!q || !out) return -1;
  if (queue_is_empty(q)) return -1;

  *out = q->buf[0];
  // shift left
  memmove(&q->buf[0], &q->buf[1], (q->len - 1) * sizeof(job_t));
  q->len--;
  return 0;
}

void queue_reschedule(job_queue_t *q, sched_policy_t new_policy) {
  if (!q) return;
  q->policy = new_policy;
  qsort(q->buf, q->len, sizeof(job_t), get_cmp(q->policy));
}

const char* policy_str(sched_policy_t p) {
  switch (p) {
    case POLICY_SJF:      return "SJF";
    case POLICY_PRIORITY: return "Priority";
    case POLICY_FCFS:
    default:              return "FCFS";
  }
}