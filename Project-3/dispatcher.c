// dispatcher.c
#include "dispatcher.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>

static int run_job_execv(const job_t *job) {
  pid_t pid = fork();
  if (pid < 0) { perror("fork"); return -1; }

  if (pid == 0) {
    char *const argv[] = { (char*)job->name, NULL };
    execv(job->name, argv);
    fprintf(stderr, "execv failed for '%s': %s\n", job->name, strerror(errno));
    _exit(127);
  }

  int status = 0;
  if (waitpid(pid, &status, 0) < 0) { perror("waitpid"); return -1; }
  return status;
}

void* dispatcher_thread(void *arg) {
  aubatch_state_t *st = (aubatch_state_t*)arg;
  if (!st) return NULL;

  while (1) {
    job_t job;
    int have_job = 0;

    pthread_mutex_lock(&st->mtx);

    while (queue_is_empty(&st->queue) && !st->shutting_down) {
      pthread_cond_wait(&st->cv_not_empty, &st->mtx);
    }

    if (st->shutting_down && queue_is_empty(&st->queue)) {
      pthread_mutex_unlock(&st->mtx);
      break;
    }

    if (!queue_is_empty(&st->queue) && queue_pop(&st->queue, &job) == 0) {
      have_job = 1;

      // Mark as running (visible to list/policy switch)
      st->has_running = 1;
      st->running_job = job;
      st->running_job.status = JOB_RUNNING;
      st->running_job.start_time = time(NULL);

      pthread_cond_signal(&st->cv_not_full);
    }

    pthread_mutex_unlock(&st->mtx);

    if (!have_job) continue;

    printf("[Dispatcher] Running job: %s (exp=%d, pri=%d)\n",
           job.name, job.expected_cpu_time, job.priority);

    int status = run_job_execv(&job);

    time_t end_time = time(NULL);

    pthread_mutex_lock(&st->mtx);
    // finalize running job record
    st->running_job.end_time = end_time;
    st->running_job.status = JOB_FINISHED;

        // --- Metrics update ---
    double turnaround = difftime(st->running_job.end_time, st->running_job.arrival_time);
    double cpu        = difftime(st->running_job.end_time, st->running_job.start_time);
    double waiting    = turnaround - cpu;

    st->total_jobs_completed++;
    st->sum_turnaround += turnaround;
    st->sum_cpu += cpu;
    st->sum_waiting += waiting;

    if (!st->has_first_start) {
      st->first_start = st->running_job.start_time;
      st->has_first_start = 1;
    }
    st->last_end = st->running_job.end_time;
    // --- End metrics update ---

    printf("[Dispatcher] Finished job: %s (status=%d, wall=%lds)\n",
           st->running_job.name, status,
           (long)(st->running_job.end_time - st->running_job.start_time));

    st->has_running = 0;
    pthread_mutex_unlock(&st->mtx);
  }

  return NULL;
}