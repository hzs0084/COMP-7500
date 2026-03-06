#include "cmd_parser.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void trim_newline(char *s) {
  if (!s) return;
  size_t n = strlen(s);
  while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) {
    s[n - 1] = '\0';
    n--;
  }
}

static test_policy_t parse_test_policy(const char *s) {
  if (!s) return TEST_POLICY_INVALID;

  if (strcmp(s, "fcfs") == 0) return TEST_POLICY_FCFS;
  if (strcmp(s, "sjf") == 0) return TEST_POLICY_SJF;
  if (strcmp(s, "priority") == 0) return TEST_POLICY_PRIORITY;

  return TEST_POLICY_INVALID;
}

void print_help(void) {
  puts("run <job> <time> <pri>: submit a job named <job>,");
  puts(" execution time is <time>,");
  puts(" priority is <pri>.");
  puts("list: display the job status.");
  puts("fcfs: change the scheduling policy to FCFS.");
  puts("sjf: change the scheduling policy to SJF.");
  puts("priority: change the scheduling policy to priority.");
  puts("test <benchmark> <policy> <num_of_jobs> <priority_levels> <min_CPU_time> <max_CPU_time> [arrival_rate]");
  puts("help -test: show usage for the test command.");
  puts("quit: exit AUbatch");
}

void print_test_help(void) {
  puts("test <benchmark> <policy> <num_of_jobs> <priority_levels> <min_CPU_time> <max_CPU_time> [arrival_rate]");
  puts(" benchmark: executable benchmark path (example: ./batch_job)");
  puts(" policy: fcfs | sjf | priority");
  puts(" num_of_jobs: number of jobs to auto-submit");
  puts(" priority_levels: random priorities will be chosen in [1, priority_levels]");
  puts(" min_CPU_time: minimum randomly generated CPU time");
  puts(" max_CPU_time: maximum randomly generated CPU time");
  puts(" arrival_rate: optional jobs/second; if omitted, the scheduler may use its default behavior");
}

int parse_command(const char *line_in, cmd_t *out) {
  if (!line_in || !out) return -1;
  memset(out, 0, sizeof(*out));
  out->test_policy = TEST_POLICY_INVALID;
  out->arrival_rate = 0.0;
  out->has_arrival_rate = 0;

  char line[256];
  snprintf(line, sizeof(line), "%s", line_in);
  trim_newline(line);

  char *p = line;
  while (*p && isspace((unsigned char)*p)) p++;

  if (*p == '\0') {
    out->type = CMD_NONE;
    return 0;
  }

  char *cmd = strtok(p, " \t");
  if (!cmd) return -1;

  if (strcmp(cmd, "help") == 0) {
    out->type = CMD_HELP;

    char *arg1 = strtok(NULL, " \t");
    if (arg1 && strcmp(arg1, "-test") == 0) {
      out->help_test_only = 1;
    }
    return 0;
  }

  if (strcmp(cmd, "list") == 0) {
    out->type = CMD_LIST;
    return 0;
  }

  if (strcmp(cmd, "fcfs") == 0) {
    out->type = CMD_FCFS;
    return 0;
  }

  if (strcmp(cmd, "sjf") == 0) {
    out->type = CMD_SJF;
    return 0;
  }

  if (strcmp(cmd, "priority") == 0) {
    out->type = CMD_PRIORITY;
    return 0;
  }

  if (strcmp(cmd, "quit") == 0) {
    out->type = CMD_QUIT;
    return 0;
  }

  if (strcmp(cmd, "run") == 0) {
    out->type = CMD_RUN;

    char *job = strtok(NULL, " \t");
    char *time_s = strtok(NULL, " \t");
    char *pri_s = strtok(NULL, " \t");

    if (!job || !time_s || !pri_s) return -1;

    snprintf(out->job_name, sizeof(out->job_name), "%s", job);
    out->cpu_time = atoi(time_s);
    out->priority = atoi(pri_s);

    if (out->cpu_time <= 0) return -1;
    return 0;
  }

  if (strcmp(cmd, "test") == 0) {
    out->type = CMD_TEST;

    char *benchmark = strtok(NULL, " \t");
    char *policy_s = strtok(NULL, " \t");
    char *num_jobs_s = strtok(NULL, " \t");
    char *priority_levels_s = strtok(NULL, " \t");
    char *min_cpu_s = strtok(NULL, " \t");
    char *max_cpu_s = strtok(NULL, " \t");
    char *arrival_rate_s = strtok(NULL, " \t");

    if (!benchmark || !policy_s || !num_jobs_s ||
        !priority_levels_s || !min_cpu_s || !max_cpu_s) {
      return -1;
    }

    snprintf(out->benchmark, sizeof(out->benchmark), "%s", benchmark);
    out->test_policy = parse_test_policy(policy_s);
    out->num_jobs = atoi(num_jobs_s);
    out->priority_levels = atoi(priority_levels_s);
    out->min_cpu_time = atoi(min_cpu_s);
    out->max_cpu_time = atoi(max_cpu_s);

    if (out->test_policy == TEST_POLICY_INVALID) return -1;
    if (out->num_jobs <= 0) return -1;
    if (out->priority_levels <= 0) return -1;
    if (out->min_cpu_time <= 0) return -1;
    if (out->max_cpu_time < out->min_cpu_time) return -1;

    if (arrival_rate_s) {
      out->arrival_rate = atof(arrival_rate_s);
      if (out->arrival_rate <= 0.0) return -1;
      out->has_arrival_rate = 1;
    }

    return 0;
  }

  return -1;
}