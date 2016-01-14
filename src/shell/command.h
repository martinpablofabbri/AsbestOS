#ifndef SRC_SHELL_COMMAND_H_
#define SRC_SHELL_COMMAND_H_

#include <stdbool.h>

/** Shell command data.
 *  Includes arguments, redirection file names, etc.
 **/

/** Command **/
typedef struct {
  /** Number of items in argv, not including NULL **/
  int argc;
  /** Argument values; always NULL-terminated **/
  char **argv;

  /** Input file name; get input from stdin if NULL **/
  char *input;
  /** Output file name; output to stdout if NULL **/
  char *output;
  /** Error file name; output to stderr if NULL **/
  char *error;

  /** Indicate if this command is a shell builtin (ie. cd, exit) **/
  bool is_builtin;
} Command;

/** Command_vec linked list **/
typedef struct Command_vec_struct {
  Command *command;
  struct Command_vec_struct *next;
} Command_vec;

#endif  // SRC_SHELL_COMMAND_H_
