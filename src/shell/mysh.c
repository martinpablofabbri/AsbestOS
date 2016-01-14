#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "read.h"
#include "eval.h"

/** Makes a prompt cstring. Caller must free it **/
char * get_prompt() {
  size_t login_size = strlen(getlogin());

  // Get current working directory for command prompt
  char *cwd = getcwd(NULL, 0);
  // Handle getcwd error.
  if (cwd == NULL) {
    fprintf(stderr, "getcwd() error.\n");
    exit(EXIT_FAILURE);
  }
  size_t cwd_size = strlen(cwd);
  char * prompt = malloc((login_size + cwd_size + 3) * sizeof(char));
  if (prompt == NULL) {
    perror("Could not allocate memory for prompt.\n");
    exit(EXIT_FAILURE);
  }
  sprintf(prompt, "%s:%s>", getlogin(), cwd);
  free(cwd);
  return prompt;
}

/* A static variable for holding the line. */
static char *line_read = (char *)NULL;

/* Read a string, and return a pointer to it.
   Returns NULL on EOF. */
char *
rl_gets ()
{
  /* If the buffer has already been allocated,
     return the memory to the free pool. */
  if (line_read)
    {
      free (line_read);
      line_read = (char *)NULL;
    }

  /* Get a line from the user. */
  char * prompt = get_prompt();
  line_read = readline (prompt);
  free(prompt);

  /* If the line has any text in it,
     save it on the history. */
  if (line_read && *line_read)
    add_history (line_read);

  return (line_read);
}

int main(int argc, char *argv[])
{
  // Shell read-evaluate loop
  for (;;) {
    // Read user input and evaluate commands
    Command_vec *commands = read_commands(rl_gets());
    eval(commands);
    free_command_vec(commands);
  }
  printf("\n");
  return 0;
}
