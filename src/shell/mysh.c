#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "read.h"
#include "eval.h"

int main(int argc, char *argv[])
{
  for (;;) {
    char *cwd = getcwd(NULL, 0);
    printf("%s:%s>", getlogin(), cwd);
    eval(read_commands());

    free(cwd);
    // TODO(jg): remove when we can input and execute an exit command
    break;
  }
  printf("\n");
  return 0;
}
