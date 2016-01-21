#ifndef KEYBOARD_H
#define KEYBOARD_H

typedef enum {
  KEY_DOWN,
  KEY_UP
} key_state;

#define NUM_KEYS 4
typedef enum {
  A_KEY,
  S_KEY,
  D_KEY,
  F_KEY
} key_name;


extern void keyboard_event(void);

void init_keyboard(void);
void get_keyboard_input();
int is_pressed (key_name k);

#endif /* KEYBOARD_H */
