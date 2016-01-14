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
    free(prv);
  }
}
