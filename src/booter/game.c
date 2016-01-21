#include "interrupts.h"
#include "keyboard.h"
#include "video.h"

/* This is the entry-point for the game! */
void c_start(void) {
  init_interrupts();
  init_timer();
  init_keyboard();
  enable_interrupts();

  /* Loop forever, so that we don't fall back into the bootloader code. */
  while (1) {
    get_keyboard_input();
  }
}

void update_game(Game *) {
  // TODO: Update game struct
}
