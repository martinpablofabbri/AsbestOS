#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "read.h"
#include "eval.h"

int main(int argc, char *argv[])
{
  // Shell read-evaluate loop
  for (;;) {
    // Get current working directory for command prompt
    char *cwd = getcwd(NULL, 0);
    // Handle getcwd error.
    if (cwd == NULL) {
      fprintf(stderr, "getcwd() error.\n");
      exit(EXIT_FAILURE);
    }

    // Print command prompt
    printf("%s:%s>", getlogin(), cwd);

    // Read user input and evaluate commands
    Command_vec *commands = read_commands();
    eval(commands);
    free_command_vec(commands);

    free(cwd);
    // TODO(jg): remove when we can input and execute an exit command
    break;
  }
  printf("\n");
  return 0;
}
