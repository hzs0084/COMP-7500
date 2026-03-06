#ifndef CMD_PARSER_H
#define CMD_PARSER_H

typedef enum {
  CMD_NONE = 0,
  CMD_HELP,
  CMD_RUN,
  CMD_LIST,
  CMD_FCFS,
  CMD_SJF,
  CMD_PRIORITY,
  CMD_TEST,
  CMD_QUIT
} cmd_type_t;

typedef enum {
  TEST_POLICY_INVALID = -1,
  TEST_POLICY_FCFS = 0,
  TEST_POLICY_SJF = 1,
  TEST_POLICY_PRIORITY = 2
} test_policy_t;

typedef struct {
  cmd_type_t type;

  // For run
  char job_name[128];
  int cpu_time;
  int priority;

  // For help -test
  int help_test_only;

  // For test
  char benchmark[128];
  test_policy_t test_policy;
  int num_jobs;
  int priority_levels;
  int min_cpu_time;
  int max_cpu_time;
  double arrival_rate;     // optional; <= 0 means "not provided"
  int has_arrival_rate;
} cmd_t;

// Parse a single line into cmd_t.
// Returns 0 on success, -1 on parse error.
int parse_command(const char *line, cmd_t *out);

// Print general help
void print_help(void);

// Print help for test command
void print_test_help(void);

#endif