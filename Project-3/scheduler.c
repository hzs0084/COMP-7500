#include "scheduler.h"
#include "cmd_parser.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static void print_job_list_locked(const aubatch_state_t *st) {
  printf("Total number of jobs in the queue: %zu\n", queue_len(&st->queue));
  printf("Scheduling Policy: %s.\n", policy_str(st->queue.policy));
  printf("Name\tCPU_Time\tPri\tArrival_time\tStatus\n");

  for (size_t i = 0; i < st->queue.len; i++) {
    const job_t *j = &st->queue.buf[i];

    char ts[32];
    struct tm tmv;
    localtime_r(&j->arrival_time, &tmv);
    strftime(ts, sizeof(ts), "%H:%M:%S", &tmv);

    const char *status =
      (j->status == JOB_WAITING) ? "Wait" :
      (j->status == JOB_RUNNING) ? "Run" : "Done";

    printf("%s\t%d\t\t%d\t%s\t\t%s\n",
           j->name, j->expected_cpu_time, j->priority, ts, status);
  }
}

void* scheduler_thread(void *arg) {
  aubatch_state_t *st = (aubatch_state_t*)arg;
  if (!st) return NULL;

  char line[256];

  while (!st->shutting_down) {
    printf("> ");
    fflush(stdout);

    if (!fgets(line, sizeof(line), stdin)) {
      // EOF (Ctrl-D) -> treat as quit
      pthread_mutex_lock(&st->mtx);
      st->shutting_down = 1;
      pthread_cond_broadcast(&st->cv_not_empty);
      pthread_mutex_unlock(&st->mtx);
      break;
    }

    cmd_t cmd;
    if (parse_command(line, &cmd) != 0) {
      puts("error: invalid command. Type 'help'.");
      continue;
    }

    if (cmd.type == CMD_NONE) continue;

    if (cmd.type == CMD_HELP) {
      print_help();
      continue;
    }

    if (cmd.type == CMD_QUIT) {
      pthread_mutex_lock(&st->mtx);
      st->shutting_down = 1;
      pthread_cond_broadcast(&st->cv_not_empty);
      pthread_mutex_unlock(&st->mtx);
      break;
    }

    if (cmd.type == CMD_LIST) {
      pthread_mutex_lock(&st->mtx);
      print_job_list_locked(st);
      pthread_mutex_unlock(&st->mtx);
      continue;
    }

    if (cmd.type == CMD_FCFS || cmd.type == CMD_SJF || cmd.type == CMD_PRIORITY) {
      sched_policy_t newp =
        (cmd.type == CMD_SJF) ? POLICY_SJF :
        (cmd.type == CMD_PRIORITY) ? POLICY_PRIORITY : POLICY_FCFS;

      pthread_mutex_lock(&st->mtx);
      queue_reschedule(&st->queue, newp);
      printf("Scheduling policy is switched to %s. All waiting jobs have been rescheduled.\n",
             policy_str(newp));
      pthread_mutex_unlock(&st->mtx);
      continue;
    }

    if (cmd.type == CMD_RUN) {
      job_t job;
      memset(&job, 0, sizeof(job));
      job.id = st->queue.next_job_id++;
      snprintf(job.name, sizeof(job.name), "%s", cmd.job_name);
      job.expected_cpu_time = cmd.cpu_time;
      job.priority = cmd.priority;
      job.arrival_time = time(NULL);
      job.status = JOB_WAITING;

      pthread_mutex_lock(&st->mtx);

      while (queue_is_full(&st->queue) && !st->shutting_down) {
        pthread_cond_wait(&st->cv_not_full, &st->mtx);
      }

      if (!st->shutting_down) {
        if (queue_insert(&st->queue, &job) == 0) {
          printf("Job %s was submitted.\n", job.name);
          printf("Total number of jobs in the queue: %zu\n", queue_len(&st->queue));
          printf("Scheduling Policy: %s.\n", policy_str(st->queue.policy));
          pthread_cond_signal(&st->cv_not_empty);
        } else {
          puts("error: queue insert failed.");
        }
      }

      pthread_mutex_unlock(&st->mtx);
      continue;
    }

    puts("error: unhandled command.");
  }

  return NULL;
}