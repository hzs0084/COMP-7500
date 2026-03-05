#include "dispatcher.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>

// Helper: run job using fork + execv + waitpid
static int run_job_execv(const job_t *job) {
  if (!job) return -1;

  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
    return -1;
  }

  if (pid == 0) {
    // Child: execv requires argv array; argv[0] is program name.
    // This skeleton runs the job with NO args. Extend as needed.
    char *const argv[] = { (char*)job->name, NULL };

    execv(job->name, argv);

    // Only reached if execv fails
    fprintf(stderr, "execv failed for '%s': %s\n", job->name, strerror(errno));
    _exit(127);
  }

  // Parent
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

    // --- Critical section: wait for job availability ---
    pthread_mutex_lock(&st->mtx);

    while (queue_is_empty(&st->queue) && !st->shutting_down) {
      pthread_cond_wait(&st->cv_not_empty, &st->mtx);
    }

    if (st->shutting_down && queue_is_empty(&st->queue)) {
      pthread_mutex_unlock(&st->mtx);
      break;
    }

    if (!queue_is_empty(&st->queue)) {
      if (queue_pop(&st->queue, &job) == 0) {
        have_job = 1;
        pthread_cond_signal(&st->cv_not_full);
      }
    }

    pthread_mutex_unlock(&st->mtx);
    // --- End critical section ---

    if (!have_job) continue;

    // Run outside lock (IMPORTANT)
    job.status = JOB_RUNNING;
    job.start_time = time(NULL);

    printf("[Dispatcher] Running job: %s (exp=%d, pri=%d)\n",
           job.name, job.expected_cpu_time, job.priority);

    int status = run_job_execv(&job);

    job.end_time = time(NULL);
    job.status = JOB_FINISHED;

    printf("[Dispatcher] Finished job: %s (status=%d, wall=%lds)\n",
           job.name, status, (long)(job.end_time - job.start_time));
  }

  return NULL;
}