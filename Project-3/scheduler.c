// scheduler.c
#include "scheduler.h"
#include "cmd_parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static long compute_expected_wait_for_new_job_locked(const aubatch_state_t *st) {
  long wait = 0;

  // Simple method allowed by FAQ: count full expected time of the running job
  if (st->has_running) {
    wait += st->running_job.expected_cpu_time;
  }

  // Add all waiting jobs ahead of the new job
  for (size_t i = 0; i < st->queue.len; i++) {
    wait += st->queue.buf[i].expected_cpu_time;
  }

  return wait;
}

static void print_job_list_locked(const aubatch_state_t *st) {
  size_t total = st->queue.len + (st->has_running ? 1 : 0);

  printf("Total number of jobs in the queue: %zu\n", total);
  printf("Scheduling Policy: %s.\n", policy_str(st->queue.policy));
  printf("%-15s %-10s %-6s %-12s %-10s\n",
       "Name", "CPU_Time", "Pri", "Arrival_time", "Progress");

  if (st->has_running) {
    const job_t *j = &st->running_job;

    char ts[32];
    struct tm tmv;
    localtime_r(&j->arrival_time, &tmv);
    strftime(ts, sizeof(ts), "%H:%M:%S", &tmv);

    printf("%-15s %-10d %-6d %-12s %-10s\n",
       j->name,
       j->expected_cpu_time,
       j->priority,
       ts,
       "Run");
  }

  for (size_t i = 0; i < st->queue.len; i++) {
    const job_t *j = &st->queue.buf[i];

    char ts[32];
    struct tm tmv;
    localtime_r(&j->arrival_time, &tmv);
    strftime(ts, sizeof(ts), "%H:%M:%S", &tmv);

    printf("%-15s %-10d %-6d %-12s %-10s\n",
       j->name,
       j->expected_cpu_time,
       j->priority,
       ts,
       "Wait");
  }
}

static sched_policy_t test_policy_to_sched(test_policy_t p) {
  switch (p) {
    case TEST_POLICY_SJF:
      return POLICY_SJF;
    case TEST_POLICY_PRIORITY:
      return POLICY_PRIORITY;
    case TEST_POLICY_FCFS:
    default:
      return POLICY_FCFS;
  }
}

static int rand_in_range(int lo, int hi) {
  if (hi <= lo) return lo;
  return lo + (rand() % (hi - lo + 1));
}

static void sleep_for_arrival_rate(double arrival_rate) {
  if (arrival_rate <= 0.0) return;

  double seconds = 1.0 / arrival_rate;
  time_t sec_part = (time_t)seconds;
  long nsec_part = (long)((seconds - (double)sec_part) * 1000000000.0);

  if (nsec_part < 0) nsec_part = 0;
  if (nsec_part >= 1000000000L) nsec_part = 999999999L;

  struct timespec ts;
  ts.tv_sec = sec_part;
  ts.tv_nsec = nsec_part;

  nanosleep(&ts, NULL);
}

static void print_test_metrics(long jobs_completed,
                               double sum_turnaround,
                               double sum_cpu,
                               double sum_waiting) {
  if (jobs_completed <= 0) {
    printf("Total number of job submitted: 0\n");
    printf("Average turnaround time: 0.00 seconds\n");
    printf("Average CPU time: 0.00 seconds\n");
    printf("Average waiting time: 0.00 seconds\n");
    printf("Throughput: 0.000 No./second\n");
    return;
  }

  double avg_turnaround = sum_turnaround / (double)jobs_completed;
  double avg_cpu = sum_cpu / (double)jobs_completed;
  double avg_waiting = sum_waiting / (double)jobs_completed;

  /*
   * Stable throughput for automated test mode:
   * jobs completed / total CPU time consumed by those jobs.
   * This avoids inflating/deflating throughput based on user typing delay.
   */
  double throughput = (sum_cpu > 0.0) ? ((double)jobs_completed / sum_cpu) : 0.0;

  printf("Total number of job submitted: %ld\n", jobs_completed);
  printf("Average turnaround time: %.2f seconds\n", avg_turnaround);
  printf("Average CPU time: %.2f seconds\n", avg_cpu);
  printf("Average waiting time: %.2f seconds\n", avg_waiting);
  printf("Throughput: %.3f No./second\n", throughput);
}

static void run_test(aubatch_state_t *st, const cmd_t *cmd) {
  if (!st || !cmd) return;

  /*
   * FAQ says test should assume an empty queue before starting.
   */
  pthread_mutex_lock(&st->mtx);
  if (st->has_running || !queue_is_empty(&st->queue)) {
    pthread_mutex_unlock(&st->mtx);
    puts("error: test requires an empty queue and no running job.");
    return;
  }
  pthread_mutex_unlock(&st->mtx);

  if (access(cmd->benchmark, F_OK) != 0) {
    printf("error: benchmark %s does not exist.\n", cmd->benchmark);
    return;
  }

  sched_policy_t newp = test_policy_to_sched(cmd->test_policy);

  /*
   * Capture baseline metrics so we can print metrics for this test only.
   */
  long base_completed;
  double base_turnaround;
  double base_cpu;
  double base_waiting;

  pthread_mutex_lock(&st->mtx);
  queue_reschedule(&st->queue, newp);
  base_completed = st->total_jobs_completed;
  base_turnaround = st->sum_turnaround;
  base_cpu = st->sum_cpu;
  base_waiting = st->sum_waiting;
  pthread_mutex_unlock(&st->mtx);

  srand((unsigned int)(time(NULL) ^ getpid()));

  for (int i = 0; i < cmd->num_jobs; i++) {
    job_t job;
    memset(&job, 0, sizeof(job));

    job.id = st->queue.next_job_id++;
    snprintf(job.name, sizeof(job.name), "%s", cmd->benchmark);
    job.expected_cpu_time = rand_in_range(cmd->min_cpu_time, cmd->max_cpu_time);
    job.priority = rand_in_range(1, cmd->priority_levels);
    job.arrival_time = time(NULL);
    job.status = JOB_WAITING;

    pthread_mutex_lock(&st->mtx);

    while (queue_is_full(&st->queue) && !st->shutting_down) {
      pthread_cond_wait(&st->cv_not_full, &st->mtx);
    }

    if (!st->shutting_down) {
      if (queue_insert(&st->queue, &job) == 0) {
        st->total_jobs_submitted++;
        pthread_cond_signal(&st->cv_not_empty);
      } else {
        pthread_mutex_unlock(&st->mtx);
        puts("error: queue insert failed during test.");
        return;
      }
    }

    pthread_mutex_unlock(&st->mtx);

    if (cmd->has_arrival_rate && i < cmd->num_jobs - 1) {
      sleep_for_arrival_rate(cmd->arrival_rate);
    }
  }

  /*
   * FAQ says test should be automatic and non-interactive while executing.
   * We wait until all jobs generated by this test finish.
   */
  while (1) {
    long completed_now;

    pthread_mutex_lock(&st->mtx);
    completed_now = st->total_jobs_completed;
    pthread_mutex_unlock(&st->mtx);

    if (completed_now >= base_completed + cmd->num_jobs) {
      break;
    }

    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 100000000L;  // 100 ms
    nanosleep(&ts, NULL);
  }

  pthread_mutex_lock(&st->mtx);
  long test_completed = st->total_jobs_completed - base_completed;
  double test_turnaround = st->sum_turnaround - base_turnaround;
  double test_cpu = st->sum_cpu - base_cpu;
  double test_waiting = st->sum_waiting - base_waiting;
  pthread_mutex_unlock(&st->mtx);

  print_test_metrics(test_completed, test_turnaround, test_cpu, test_waiting);
}

void* scheduler_thread(void *arg) {
  aubatch_state_t *st = (aubatch_state_t*)arg;
  if (!st) return NULL;

  char line[256];

  while (!st->shutting_down) {
    printf("> ");
    fflush(stdout);

    if (!fgets(line, sizeof(line), stdin)) {
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
      if (cmd.help_test_only) {
        print_test_help();
      } else {
        print_help();
      }
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
      size_t waiting = st->queue.len;
      queue_reschedule(&st->queue, newp);
      printf("Scheduling policy is switched to %s. All the %zu waiting jobs\nhave been rescheduled.\n",
             policy_str(newp), waiting);
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

      if (access(job.name, F_OK) != 0) {
        printf("error: job %s does not exist.\n", job.name);
        continue;
      }

      pthread_mutex_lock(&st->mtx);

      while (queue_is_full(&st->queue) && !st->shutting_down) {
        pthread_cond_wait(&st->cv_not_full, &st->mtx);
      }

      if (!st->shutting_down) {
        long expected_wait = compute_expected_wait_for_new_job_locked(st);

        if (queue_insert(&st->queue, &job) == 0) {
          printf("Job %s was submitted.\n", job.name);
          st->total_jobs_submitted++;

          size_t total = st->queue.len + (st->has_running ? 1 : 0);
          printf("Total number of jobs in the queue: %zu\n", total);
          printf("Expected waiting time: %ld seconds\n", expected_wait);
          printf("Scheduling Policy: %s.\n", policy_str(st->queue.policy));

          pthread_cond_signal(&st->cv_not_empty);
        } else {
          puts("error: queue insert failed.");
        }
      }

      pthread_mutex_unlock(&st->mtx);
      continue;
    }

    if (cmd.type == CMD_TEST) {
      run_test(st, &cmd);
      continue;
    }

    puts("error: unhandled command.");
  }

  return NULL;
}