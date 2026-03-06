#ifndef JOB_H
#define JOB_H

#include <time.h>

typedef enum {
  JOB_WAITING = 0,
  JOB_RUNNING = 1,
  JOB_FINISHED = 2
} job_status_t;

typedef struct {
  int id;

  // User-provided
  char name[128];              // job "name" (executable path or name)
  int expected_cpu_time;       // used for SJF / wait estimates
  int priority;                // used for Priority policy

  // Times for metrics / list display
  time_t arrival_time;
  time_t start_time;
  time_t end_time;

  job_status_t status;
} job_t;

#endif