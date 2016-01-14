#include "command.h"

#include <stdlib.h>

/**
 * Clean up the command linked list
 */
void free_command_vec (Command_vec *cv) {
  Command_vec *cur = cv;
  while (cur) {
    Command_vec *prv = cur;
    cur = cur->next;

    Command *cmd = prv->command;
    int i;
    for (i=0; i<cmd->argc-1; i++) {
      free(cmd->argv[i]);
    }
    free(cmd->argv);
    if (cmd->input)
      free(cmd->input);
    if (cmd->output)
      free(cmd->output);
    if (cmd->error)
      free(cmd->error);
    free(cmd);
    free(prv);
  }
}
