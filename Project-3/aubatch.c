#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "aubatch.h"
#include "scheduler.h"
#include "dispatcher.h"
#include "cmd_parser.h"

#define DEFAULT_QUEUE_CAP 64

int main(void) {
  aubatch_state_t st;
  st.shutting_down = 0;
  st.has_running = 0;
  st.total_jobs_submitted = 0;
  st.total_jobs_completed = 0;
  st.sum_turnaround = st.sum_cpu = st.sum_waiting = 0.0;
  st.has_first_start = 0;

  if (queue_init(&st.queue, DEFAULT_QUEUE_CAP) != 0) {
    fprintf(stderr, "Failed to init queue\n");
    return 1;
  }

  pthread_mutex_init(&st.mtx, NULL);
  pthread_cond_init(&st.cv_not_empty, NULL);
  pthread_cond_init(&st.cv_not_full, NULL);

  puts("Welcome to Hemant's batch job scheduler Version 1.0");
  puts("Type 'help' to find more about AUbatch commands.");

  pthread_t th_scheduler, th_dispatcher;

  if (pthread_create(&th_dispatcher, NULL, dispatcher_thread, &st) != 0) {
    fprintf(stderr, "Failed to create dispatcher thread\n");
    return 1;
  }
  if (pthread_create(&th_scheduler, NULL, scheduler_thread, &st) != 0) {
    fprintf(stderr, "Failed to create scheduler thread\n");
    return 1;
  }

  // Wait for scheduler to exit (user quit). Then wait for dispatcher to finish queue.
  pthread_join(th_scheduler, NULL);
  pthread_join(th_dispatcher, NULL);

// Print performance metrics (quit output)
  if (st.total_jobs_completed > 0) {
    double avg_turnaround = st.sum_turnaround / st.total_jobs_completed;
    double avg_cpu        = st.sum_cpu / st.total_jobs_completed;
    double avg_wait       = st.sum_waiting / st.total_jobs_completed;

    double duration = st.has_first_start ? difftime(st.last_end, st.first_start) : 0.0;
    double throughput = (duration > 0.0) ? ((double)st.total_jobs_completed / duration) : 0.0;

    printf("Total number of job submitted: %ld\n", st.total_jobs_submitted);
    printf("Average turnaround time: %.2f seconds\n", avg_turnaround);
    printf("Average CPU time: %.2f seconds\n", avg_cpu);
    printf("Average waiting time: %.2f seconds\n", avg_wait);
    printf("Throughput: %.3f No./second\n", throughput);
  }

  pthread_cond_destroy(&st.cv_not_empty);
  pthread_cond_destroy(&st.cv_not_full);
  pthread_mutex_destroy(&st.mtx);
  queue_destroy(&st.queue);

  puts("AUbatch exiting.");
  return 0;
}