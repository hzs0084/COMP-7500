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

  pthread_cond_destroy(&st.cv_not_empty);
  pthread_cond_destroy(&st.cv_not_full);
  pthread_mutex_destroy(&st.mtx);
  queue_destroy(&st.queue);

  puts("AUbatch exiting.");
  return 0;
}