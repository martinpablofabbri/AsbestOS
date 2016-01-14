#ifndef SRC_SHELL_EVAL_H_
#define SRC_SHELL_EVAL_H_

#include <stdio.h>

#include "command.h"

/* Store all the information needed to
  S execute a command
*/
typedef struct {
  Command *cmd;
  int input;
  int output;
  int error;
  // Sometimes the child process gets an
  // extra fd it has to close
  int extra_fd;
  int pid;
} cmd_exec_info;

void eval(Command_vec* cv);

#endif  // SRC_SHELL_EVAL_H_
