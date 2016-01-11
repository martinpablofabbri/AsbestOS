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
  int pid;
} cmd_exec_info;

void eval(Command_vec cv);
void eval_tests();

#endif  // SRC_SHELL_EVAL_H_
