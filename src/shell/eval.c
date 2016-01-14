#include "eval.h"

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/stat.h>
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
  int fd;
  if (c->input) {
    fd = open(c->input, O_RDONLY);
    // TODO(keegan): handle error case
  } else {
    // TODO(keegan): handle error case in dup
    fd = dup(STDIN_FILENO);
  }
  return fd;
}

/**
 * Tie the final pipe to either stdout or where specified
 *
 * Returns the file descriptor of the final stream
 * on success, -1 on failure.
 */
int setup_last_pipe (Command *c) {
  // TODO(keegan): Handle dup error case
  int fd;
  if (c->output) {
    fd = open(c->output, O_WRONLY | O_CREAT);
    // TODO(keegan): handle error case
  } else {
    fd = dup(STDOUT_FILENO);
  }
  return fd;
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
    // TODO(keegan): handle error case
    close(c->input);
    close(c->output);
    close(c->error);
    if (c->extra_fd != -1) {
      close(c->extra_fd);
    }

    // execvp the specified function
    char **argv = c->cmd->argv;
    execvp(argv[0], argv);
    // TODO(keegan): Error out here
  }
}

/** Evaluate all commands in Command_vec **/
void eval(Command_vec cv) {
  /* All commands are connected with pipes.
     The first command is hooked up to either
     STDIN or something as specified in the command.
     The last command is hooked up to either
     STDOUT or something as specified in the command.
  */
  // TODO(keegan): handle case where there are no commands?

  Command_vec *cur = &cv;
  int input_pipe_fd = setup_first_pipe (cur->command);
  int bridge_pipe_fd[2];
  // TODO(keegan): handle errors here
  while (cur != NULL) {
    Command *command = cur->command;
    cmd_exec_info c;
    c.cmd = command;
    c.input = input_pipe_fd;
    if (cur->next != NULL) {
      // Make pipe that will bridge between commands
      if (pipe(bridge_pipe_fd) != 0) {
	// TODO(keegan): handle error
      }
      c.output = bridge_pipe_fd[1];
      c.extra_fd = bridge_pipe_fd[0];
      input_pipe_fd = bridge_pipe_fd[0];
    } else {
      // This is the last command
      int final_stream_fd = setup_last_pipe(command);
      // TODO(keegan): handle error
      c.output = final_stream_fd;
      c.extra_fd = -1;
    }
    c.error = dup(STDERR_FILENO);
    // TODO(keegan): handle error

    start_execution(&c);
    // TODO(keegan): Add error handling for start_command

    cur = cur->next;
  }

  cur = &cv;
  while (cur != NULL) {
    // TODO(keegan): Add error handling
    wait(NULL);
    cur = cur->next;
  }
}

void eval_tests () {
  // Runs tests related to the evaluator

  // Set up a command vector
  Command_vec cv1, cv2;
  Command c[2];
  c[0].argc = 2;
  char **argv = (char **)malloc(sizeof(char*)*(c[0].argc + 1));
  argv[0] = "grep";
  argv[1] = "lol";
  argv[2] = NULL;
  c[0].argv = argv;
  c[0].input = "foo.txt";
  c[0].output = c[0].error = NULL;
  c[0].is_builtin = false;

  c[1].argc = 1;
  argv = (char **)malloc(sizeof(char*)*(c[1].argc + 1));
  argv[0] = "sort";
  argv[1] = NULL;
  c[1].argv = argv;
  c[1].input = c[1].output = c[1].error = NULL;
  //c[1].output = "bar.txt";
  c[1].is_builtin = false;

  cv1.command = &c[0];
  cv2.command = &c[1];
  cv1.next = &cv2;
  cv2.next = NULL;

  eval(cv1);
}
