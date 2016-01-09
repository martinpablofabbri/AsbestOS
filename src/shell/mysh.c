#include <stdio.h>
#include "read.h"
#include "eval.h"

int main(int argc, char *argv[])
{
  // TODO(jg): remove
  printf("Entered main in mysh.c\n");

  for (;;) {
    eval(read_commands());
    // TODO(jg): remove when we can input and execute an exit command
    break;
  }

  return 0;
}
