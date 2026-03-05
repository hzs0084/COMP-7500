#include "cmd_parser.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static void trim_newline(char *s) {
  if (!s) return;
  size_t n = strlen(s);
  while (n > 0 && (s[n-1] == '\n' || s[n-1] == '\r')) {
    s[n-1] = '\0';
    n--;
  }
}

void print_help(void) {
  puts("run <job> <time> <pri>: submit a job named <job>,");
  puts(" execution time is <time>,");
  puts(" priority is <pri>.");
  puts("list: display the job status.");
  puts("fcfs: change the scheduling policy to FCFS.");
  puts("sjf: change the scheduling policy to SJF.");
  puts("priority: change the scheduling policy to priority.");
  puts("quit: exit AUbatch");
}

int parse_command(const char *line_in, cmd_t *out) {
  if (!line_in || !out) return -1;
  memset(out, 0, sizeof(*out));

  char line[256];
  snprintf(line, sizeof(line), "%s", line_in);
  trim_newline(line);

  // Skip leading spaces
  char *p = line;
  while (*p && isspace((unsigned char)*p)) p++;
  if (*p == '\0') { out->type = CMD_NONE; return 0; }

  // Tokenize
  char *cmd = strtok(p, " \t");
  if (!cmd) return -1;

  if (strcmp(cmd, "help") == 0) { out->type = CMD_HELP; return 0; }
  if (strcmp(cmd, "list") == 0) { out->type = CMD_LIST; return 0; }
  if (strcmp(cmd, "fcfs") == 0) { out->type = CMD_FCFS; return 0; }
  if (strcmp(cmd, "sjf") == 0) { out->type = CMD_SJF; return 0; }
  if (strcmp(cmd, "priority") == 0) { out->type = CMD_PRIORITY; return 0; }
  if (strcmp(cmd, "quit") == 0) { out->type = CMD_QUIT; return 0; }

  if (strcmp(cmd, "run") == 0) {
    out->type = CMD_RUN;

    char *job = strtok(NULL, " \t");
    char *time_s = strtok(NULL, " \t");
    char *pri_s  = strtok(NULL, " \t");

    if (!job || !time_s || !pri_s) return -1;

    snprintf(out->job_name, sizeof(out->job_name), "%s", job);
    out->cpu_time = atoi(time_s);
    out->priority = atoi(pri_s);

    if (out->cpu_time <= 0) return -1;
    // priority can be any int; you can add bounds later
    return 0;
  }

  // Unknown command
  return -1;
}