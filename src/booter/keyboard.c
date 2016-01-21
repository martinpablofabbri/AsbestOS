#include "handlers.h"
#include "interrupts.h"
#include "keyboard.h"
#include "ports.h"

/* This is the IO port of the PS/2 controller, where the keyboard's scan
 * codes are made available.  Scan codes can be read as follows:
 *
 *     unsigned char scan_code = inb(KEYBOARD_PORT);
 *
 * Most keys generate a scan-code when they are pressed, and a second scan-
 * code when the same key is released.  For such keys, the only difference
 * between the "pressed" and "released" scan-codes is that the top bit is
 * cleared in the "pressed" scan-code, and it is set in the "released" scan-
 * code.
 *
 * A few keys generate two scan-codes when they are pressed, and then two
 * more scan-codes when they are released.  For example, the arrow keys (the
 * ones that aren't part of the numeric keypad) will usually generate two
 * scan-codes for press or release.  In these cases, the keyboard controller
 * fires two interrupts, so you don't have to do anything special - the
 * interrupt handler will receive each byte in a separate invocation of the
 * handler.
 *
 * See http://wiki.osdev.org/PS/2_Keyboard for details.
 */
#define KEYBOARD_PORT 0x60
#define SC_BUFFER_SIZE 0x40

/* TODO:  You can create static variables here to hold keyboard state.
 *        Note that if you create some kind of circular queue (a very good
 *        idea, you should declare it "volatile" so that the compiler knows
 *        that it can be changed by exceptional control flow.
 *
 *        Also, don't forget that interrupts can interrupt *any* code,
 *        including code that fetches key data!  If you are manipulating a
 *        shared data structure that is also manipulated from an interrupt
 *        handler, you might want to disable interrupts while you access it,
 *        so that nothing gets mangled...
 */

static volatile unsigned char scancode_buffer[SC_BUFFER_SIZE];
// The head points to the oldest element in the buffer
unsigned head_idx;
unsigned sc_len;

key_state keyboard_state[NUM_KEYS];

void enqueue (unsigned char scan_code) {
  disable_interrupts();
  
  if (sc_len == SC_BUFFER_SIZE) {
    // Overwrite the old head, advance head by one
    scancode_buffer[head_idx] = scan_code;
    head_idx = (head_idx + 1) % SC_BUFFER_SIZE;
  } else {
    // Add to the tail
    unsigned tail_idx = (head_idx + sc_len) % SC_BUFFER_SIZE;
    scancode_buffer[tail_idx] = scan_code;
    sc_len++;
  }

  enable_interrupts();
}

/**
 * Dequeue from the circular buffer. Returns garbage if the buffer
 * is empty.
 */
unsigned char dequeue () {
  disable_interrupts();

  // Read the head, advance head by one
  unsigned char head = scancode_buffer[head_idx];
  head_idx = (head_idx + 1) % SC_BUFFER_SIZE;
  if (sc_len)
    sc_len--;

  enable_interrupts();
  return head;
}

// Return 1 if the buffer is empty, 0 otherwise
int is_empty () {
  return (sc_len == 0) ? 1 : 0;
}

void init_keyboard(void) {
  head_idx = 0;
  sc_len = 0;

  int i;
  for (i = 0; i < NUM_KEYS; i++) {
    keyboard_state[i] = KEY_UP;
  }

  install_interrupt_handler(KEYBOARD_INTERRUPT, keyboard_handler);
}

void keyboard_event () {
  unsigned char scan_code = inb(KEYBOARD_PORT);

  // Load the scan code into the circular buffer
  enqueue(scan_code);
}

/**
 * Sets the internal state of the keyboard by analyzing
 * the scancodes. Called from the main game loop.
 */
void get_keyboard_input () {
  while (!is_empty ()) {
    unsigned char scan_code = dequeue();
    switch (scan_code & 0x7f){
    case 0x1E:
      keyboard_state[(unsigned)A_KEY] = (scan_code & 0x80) ? KEY_UP : KEY_DOWN;
      break;
    }
  }
}

/**
 * Returns the state of the specified key.
 *   1 - the key is pressed
 *   0 - the key is not pressed
 */
int is_pressed (key_name k) {
  return (keyboard_state[(unsigned)k] == KEY_DOWN) ? 1 : 0;
}
