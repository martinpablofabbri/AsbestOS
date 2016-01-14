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
  cmd->argv = (char**) malloc(sizeof(char*) * (numWords + 1));
  int sizeWord = 0;
  char* tmpBuffer;
  int i;
  while (1)
  {

    // Read in word
    while (string[i] != ' ' && string[i] != '\0' && string[i] != '|') {
      i++;
    }

    // End of word here
    tmpBuffer = (char*) malloc(sizeof(char) * (i - sizeWord) + 1);
    memcpy(tmpBuffer, &string[sizeWord], (i - sizeWord));
    tmpBuffer[i - sizeWord] = '\0';
    sizeWord = i;
    numWords++;
    cmd->argv = (char**) realloc(cmd->argv, sizeof(char*) * (numWords + 1));
    cmd->argv[numWords - 1] = tmpBuffer;
    cmd->argc = numWords;
    cmd->argv[numWords] = NULL;

    // sizeWord is the idx of first character of the next token
    // chomp whitespace

    // If there's more to the command
    while (string[sizeWord] == ' ') {sizeWord++;i++;}

    // If we need another command
    if (string[i] == '|') {
      /*      if (strcmp(cmd->argv[0], "cd") == 0)
      {
        cmd->is_builtin = true;
      }
      else if (strcmp(cmd->argv[0], "chdir") == 0)
      {
        cmd->is_builtin = true;
      }
      else if (strcmp(cmd->argv[0], "exit") == 0)
      {
        cmd->is_builtin = true;
	}*/
      nextCV->command = cmd;
      cmd = (Command *) malloc(sizeof(Command));
      Command_vec *cv = (Command_vec *) malloc(sizeof(Command_vec*));
      nextCV->next = cv;
      cmd->input = NULL;
      cmd->output = NULL;
      cmd->error = NULL;
      numWords = 0;
      cmd->argv = (char**) malloc(sizeof(char*) * (numWords + 1));

      nextCV = nextCV->next;
      // Consume |
      sizeWord++; i++;
      while (string[sizeWord] == ' ') {sizeWord++;i++;}
      if (string[i] == '\0')
	break;
    }

    if (string[i] == '\0') {
      nextCV->command = cmd;
      nextCV->next = NULL;
      break;
    }
  }
  return cv;
}
