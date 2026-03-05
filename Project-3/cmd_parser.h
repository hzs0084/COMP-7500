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
  CMD_QUIT
} cmd_type_t;

typedef struct {
  cmd_type_t type;

  // For run
  char job_name[128];
  int cpu_time;
  int priority;
} cmd_t;

// Parse a single line into cmd_t.
// Returns 0 on success, -1 on parse error.
int parse_command(const char *line, cmd_t *out);

// Print help
void print_help(void);

#endif