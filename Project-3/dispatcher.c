// dispatcher.c
#include "dispatcher.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

static int run_job_execv(const job_t *job) {
  if (!job) return -1;

  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
    return -1;
  }

  if (pid == 0) {
    // Child: execv(job->name, [job->name, "<seconds>", NULL])
    char time_arg[32];
    snprintf(time_arg, sizeof(time_arg), "%d", job->expected_cpu_time);

    char *const argv[] = { (char*)job->name, time_arg, NULL };
    execv(job->name, argv);

    // Only runs if execv fails
    fprintf(stderr, "execv failed for '%s': %s\n", job->name, strerror(errno));
    _exit(127);
  }

  // Parent: wait for completion
  int status = 0;
  if (waitpid(pid, &status, 0) < 0) {
    perror("waitpid");
    return -1;
  }
  return status;
}

void* dispatcher_thread(void *arg) {
  aubatch_state_t *st = (aubatch_state_t*)arg;
  if (!st) return NULL;

  while (1) {
    job_t job;
    int have_job = 0;

    // ---- Critical section: wait for a job and pop it ----
    pthread_mutex_lock(&st->mtx);

    while (queue_is_empty(&st->queue) && !st->shutting_down) {
      pthread_cond_wait(&st->cv_not_empty, &st->mtx);
    }

    // If quitting and no more waiting jobs, exit dispatcher
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

      // Wake scheduler if it was blocked on "queue full"
      pthread_cond_signal(&st->cv_not_full);
    }

    pthread_mutex_unlock(&st->mtx);
    // ---- End critical section ----

    if (!have_job) continue;

    // Log to stderr so it doesn't mess up the interactive prompt output
    fprintf(stderr, "[Dispatcher] Running job: %s (exp=%d, pri=%d)\n",
            job.name, job.expected_cpu_time, job.priority);

    // Run outside lock
    int status = run_job_execv(&job);
    time_t end_time = time(NULL);

    // Values copied for printing after unlock
    char name_copy[128];
    long wall_seconds = 0;

    // ---- Critical section: finalize running job + metrics ----
    pthread_mutex_lock(&st->mtx);

    st->running_job.end_time = end_time;
    st->running_job.status = JOB_FINISHED;

    // Metrics update
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

    // Copy for printing after unlock (avoid holding lock during I/O)
    snprintf(name_copy, sizeof(name_copy), "%s", st->running_job.name);
    wall_seconds = (long)(st->running_job.end_time - st->running_job.start_time);

    st->has_running = 0;

    pthread_mutex_unlock(&st->mtx);
    // ---- End critical section ----

    fprintf(stderr, "[Dispatcher] Finished job: %s (status=%d, wall=%lds)\n",
            name_copy, status, wall_seconds);
  }

  return NULL;
}