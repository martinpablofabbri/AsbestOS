#include "eval.h"

#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>

#include "command.h"

/**
 * Return a new file descriptor that should
 * be used as input to the first command.
 *
 * Returns -1 on error
 */
int setup_first_pipe (Command *c) {
  // TODO(keegan): Make this one end of a pipe istead of stdin
  // TODO(keegan): handle error case in dup
  return dup(STDIN_FILENO);
}

/**
 * Tie the final pipe to either stdout or where specified
 *
 * Returns the file descriptor of the final stream
 * on success, -1 on failure.
 */
int setup_last_pipe (Command *c) {
  // TODO(keegan): Handle dup error case
  return dup(STDOUT_FILENO);
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
    c->pid = p;
    // Close the file descriptors we don't need any more
    if (close(c->input) != 0) {
      // TODO(keegan): handle error
    }
    if (close(c->output) != 0) {
      // TODO(keegan): handle error
    }
    if (close(c->error) != 0) {
      // TODO(keegan): handle error
    }
  } else {
    // Child
    // Tie together input, output, and stderr
    // TODO(keegan): handle error case
    dup2(c->input, STDIN_FILENO);
    dup2(c->output, STDOUT_FILENO);
    dup2(c->error, STDERR_FILENO);
    // TODO(keegan): remove the file descriptors we don't need

    // execvp the specified function
    char **argv = c->cmd->argv;
    execvp(argv[0], argv);
    // TODO(keegan): Error out here
  }
}

/** Evaluate all commands in Command_vec **/
void eval(Command_vec cv) {
  unsigned i;
  /* All commands are connected with pipes.
     The first command is hooked up to either
     STDIN or something as specified in the command.
     The last command is hooked up to either
     STDOUT or something as specified in the command.
  */
  if (cv.size == 0) {
    return;
  }

  int input_pipe_fd = setup_first_pipe (&cv.commands[0]);
  int bridge_pipe_fd[2];
  // TODO(keegan): handle errors here
  for (i=0; i < cv.size; i++) {
    Command *command = &cv.commands[i];
    cmd_exec_info c;
    c.cmd = command;
    c.input = input_pipe_fd;
    if (i < cv.size - 1) {
      // Make pipe that will bridge between commands
      if (pipe(bridge_pipe_fd) != 0) {
	// TODO(keegan): handle error
      }
      c.output = bridge_pipe_fd[1];
      input_pipe_fd = bridge_pipe_fd[0];
    } else {
      // This is the last command
      int final_stream_fd = setup_last_pipe(command);
      // TODO(keegan): handle error
      c.output = final_stream_fd;
    }
    c.error = dup(STDERR_FILENO);
    // TODO(keegan): handle error

    start_execution(&c);
    // TODO(keegan): Add error handling for start_command
  }

  for (i=0; i < cv.size; i++) {
    // TODO(keegan): Add error handling
    wait(NULL);
  }
}

void eval_tests () {
  // Runs tests related to the evaluator

  // Set up a command vector
  Command_vec cv;
  Command c[2];
  c[0].argc = 2;
  char **argv = (char **)malloc(sizeof(char*)*(c[0].argc + 1));
  argv[0] = "ls";
  argv[1] = "-a";
  argv[2] = NULL;
  c[0].argv = argv;
  c[0].input = c[0].output = c[0].error = NULL;
  c[0].is_builtin = false;

  c[1].argc = 1;
  argv = (char **)malloc(sizeof(char*)*(c[1].argc + 1));
  argv[0] = "sort";
  argv[1] = NULL;
  c[1].argv = argv;
  c[1].input = c[1].output = c[1].error = NULL;
  c[1].is_builtin = false;

  cv.size = 2;
  cv.commands = c;

  eval(cv);
}
