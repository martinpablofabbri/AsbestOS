#include "eval.h"

#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>

#include "command.h"

/**
 * Fill in the file descriptors of the command
 * execution structure
 *
 * Returns true on success, false otherwise.
 */
bool set_stream_fds (cmd_exec_info *c) {
  c->input = STDIN_FILENO;
  c->output = STDOUT_FILENO;
  c->error = STDERR_FILENO;
  return true;
}

/**
 * Begin execution of the specified command.
 * Fills in the PID of the child process in the struct.
 * If fork fails, 
 */
void start_execution (cmd_exec_info *c) {
  int p = fork();
  if (p == -1) {
    // TODO(keegan): Print out detailed error msg
  } else if (p) {
    // Parent
    printf("The parent is executing!\n");
    c->pid = p;
  } else {
    // Child
    // execvp the specified function
    char **argv = c->cmd->argv;
    execvp(argv[0], argv);
    // TODO(keegan): Error out here
  }
}

/** Evaluate all commands in Command_vec **/
void eval(Command_vec cv) {
  /* Basic steps to evaluate:
       Loop over the cv datastructure
         Get necessary file descriptors
	 fork
           child - replace stdin and stdout with file descriptors
	   parent - do any glue necessary
	          - wait for child
  */
  unsigned i;
  for (i=0; i < cv.size; i++) {
    printf("Eval %d\n", i);
    Command *command = &cv.commands[i];
    cmd_exec_info c;
    c.cmd = command;
    
    set_stream_fds(&c);
    start_execution(&c);
    // TODO(keegan): Add error handling for start_command
    // and wait()
    wait(NULL);
  }
}

void eval_tests () {
  // Runs tests related to the evaluator

  // Set up a command vector
  Command_vec cv;
  Command c;
  c.argc = 1;
  char **argv = (char **)malloc(sizeof(char*)*(c.argc + 1));
  argv[0] = "ls";
  argv[1] = NULL;
  c.argv = argv;
  c.input = c.output = c.error = NULL;
  c.is_builtin = false;

  cv.size = 1;
  cv.commands = &c;

  eval(cv);
}
