#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "command.h"

Command_vec* read_commands(char* string) {

  Command_vec *cv = (Command_vec *) malloc(sizeof(Command_vec*));


  // Set up cv
  // return cv;

  Command_vec *nextCV = cv;
//---------
  Command *cmd = (Command *) malloc(sizeof(Command));
  cmd->input = NULL;
  cmd->output = NULL;
  cmd->error = NULL;

  int numWords = 0;
  cmd->argv = (char**) malloc(sizeof(char*) * numWords);
  int sizeWord = 0;
  char* tmpBuffer;
  for (int i = 0; i < strlen(string); i++)
  {
    if (strcmp(&string[i], " "))
    {
      // sizeWord++;
      tmpBuffer = (char*) malloc(sizeof(char) * (i - sizeWord) + 1);
      memcpy(tmpBuffer, &string[sizeWord], (i - sizeWord) + 1);
      sizeWord = i;
      numWords++;
      cmd->argv = (char**) realloc(cmd->argv, sizeof(char*) * numWords);
      cmd->argv[numWords] = tmpBuffer;
    }
    else if (strcmp(&string[i], "|"))
    {
      cmd->argc = numWords;
      if (strcmp(cmd->argv[0], "cd"))
      {
        cmd->is_builtin = true;
      }
      else if (strcmp(cmd->argv[0], "chdir"))
      {
        cmd->is_builtin = true;
      }
      else if (strcmp(cmd->argv[0], "exit"))
      {
        cmd->is_builtin = true;
      }
    nextCV->command = cmd;
    nextCV = nextCV->next;
    cmd = (Command *) malloc(sizeof(Command));
    cmd->input = NULL;
    cmd->output = NULL;
    cmd->error = NULL;
    }
  }
  return cv;
}
