#include "eval.h"

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
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
  } else {
    fd = dup(STDIN_FILENO);
  }
  if (fd == -1) {
    perror("Unable to set up input");
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
  int fd;
  if (c->output) {
    fd = open(c->output, O_WRONLY | O_CREAT);
  } else {
    fd = dup(STDOUT_FILENO);
  }
  if (fd == -1) {
    perror("Unable to setup output");
  }
  return fd;
}

/**
 * Begin execution of the specified command.
 * Fills in the PID of the child process in the struct.
 * 
 * Returns the PID of the child on success, and -1 on error
 */
int start_execution (cmd_exec_info *c) {
  int p = fork();
  if (p == -1) {
    perror("There was a problem forking the main shell");
    return -1;
  } else if (p) {
    // Parent
    c->pid = p;
    // Close the file descriptors we don't need any more
    // If we ever get an error, then report it but do not
    // halt.
    if (close(c->input) != 0) {
      perror("Problem closing input stream");
    }
    if (close(c->output) != 0) {
      perror("Problem closing output stream");
    }
    if (close(c->error) != 0) {
      perror("Problem closing error stream");
    }
    return p;
  } else {
    // Child
    // Tie together input, output, and stderr
    // If any has an error, die.
    if (dup2(c->input, STDIN_FILENO) == -1) {
      perror("Error duplicating command input");
      exit(-1);
    }
    if (dup2(c->output, STDOUT_FILENO) == -1) {
      perror("Error duplicating command output");
      exit(-1);
    }
    if (dup2(c->error, STDERR_FILENO) == -1) {
      perror("Error duplicating command error stream");
      exit(-1);
    }

    if(close(c->input) == -1) {
      perror("Error cleaning up file descriptors in child");
    }
    if(close(c->output) == -1) {
      perror("Error cleaning up file descriptors in child");
    }
    if(close(c->error) == -1) {
      perror("Error cleaning up file descriptors in child");
    }
    if (c->extra_fd != -1) {
      if(close(c->extra_fd) == -1) {
	perror("Error cleaning up file descriptors in child");
      }
    }

    // execvp the specified function
    char **argv = c->cmd->argv;
    execvp(argv[0], argv);
    perror("Error calling execvp");

    exit(-1);
  }
}

/** Evaluate all commands in Command_vec **/
void eval(Command_vec* cv) {
  /* All commands are connected with pipes.
     The first command is hooked up to either
     STDIN or something as specified in the command.
     The last command is hooked up to either
     STDOUT or something as specified in the command.
  */
  if (cv == NULL)
    return;

  Command_vec *cur = cv;
  int bridge_pipe_fd[2];
  int input_pipe_fd = setup_first_pipe (cur->command);
  if (input_pipe_fd == -1) {
    // Nothing to do here, nothing to clean up.
    return;
  }

  unsigned num_children = 0;
  while (cur != NULL) {
    Command *command = cur->command;
    cmd_exec_info c;
    c.cmd = command;
    c.input = input_pipe_fd;
    c.error = dup(STDERR_FILENO);
    if (c.error == -1) {
      perror("Error setting up error stream output");
      // clean up the input pipe
      if(close(input_pipe_fd) == -1) {
	perror("Error cleaning up file descriptors");
      }
      break;
    }
    if (cur->next != NULL) {
      // Make pipe that will bridge between commands
      if (pipe(bridge_pipe_fd) != 0) {
	perror("Error setting up pipe between commands");
	// Clean up file descriptors
	if(close(input_pipe_fd) == -1) {
	  perror("Error cleaning up file descriptors");
	}
	if(close(c.error) == -1) {
	  perror("Error cleaning up file descriptors");
	}
	break; // Go to wait for processes to end
      }
      c.output = bridge_pipe_fd[1];
      c.extra_fd = bridge_pipe_fd[0];
      input_pipe_fd = bridge_pipe_fd[0];
    } else {
      // This is the last command
      int final_stream_fd = setup_last_pipe(command);
      if (final_stream_fd == -1) {
	// Clean up file descriptors
	if(close(input_pipe_fd) == -1) {
	  perror("Error cleaning up file descriptors");
	}
	if(close(c.error) == -1) {
	  perror("Error cleaning up file descriptors");
	}
	break; // Go to wait for processes to end
      }
      c.output = final_stream_fd;
      c.extra_fd = -1;
    }

    int pid = start_execution(&c);
    if (pid == -1) {
	// Clean up file descriptors
	if(close(input_pipe_fd) == -1) {
	  perror("Error cleaning up file descriptors");
	}
	if(close(c.error) == -1) {
	  perror("Error cleaning up file descriptors");
	}
	break; // Go to wait for processes to end
    }

    num_children += 1;
    cur = cur->next;
  }

  /* Clean up processes */
  unsigned i;
  for (i = 0; i < num_children; i++) {
    if (wait(NULL) == -1) {
      perror("Something went wrong while waiting for children to die");
    }
  }
}
