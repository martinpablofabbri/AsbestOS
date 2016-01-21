#include "interrupts.h"
#include "keyboard.h"
#include "video.h"

/* This is the entry-point for the game! */
void c_start(void) {
    /* TODO:  You will need to initialize various subsystems here.  This
     *        would include the interrupt handling mechanism, and the various
     *        systems that use interrupts.  Once this is done, you can call
     *        enable_interrupts() to start interrupt handling, and go on to
     *        do whatever else you decide to do!
     */
  init_interrupts();
  init_timer();
  init_keyboard();
  enable_interrupts();

#define VIDEO_BUFFER ((void *) 0xB8000)

  /* Loop forever, so that we don't fall back into the bootloader code. */
  while (1) {
    get_keyboard_input();
    ((char*)VIDEO_BUFFER)[0] = is_pressed(A_KEY) ? 'A' : 'a';
    ((char*)VIDEO_BUFFER)[2] = is_pressed(S_KEY) ? 'S' : 's';
    ((char*)VIDEO_BUFFER)[4] = is_pressed(D_KEY) ? 'D' : 'd';
    ((char*)VIDEO_BUFFER)[6] = is_pressed(F_KEY) ? 'F' : 'f';
    ((char*)VIDEO_BUFFER)[8] = ' ';
    ((char*)VIDEO_BUFFER)[9] = is_pressed(SPACE_KEY) ? (WHITE << 4) : (BLACK << 4);
    ((char*)VIDEO_BUFFER)[10] = is_pressed(J_KEY) ? 'J' : 'j';
    ((char*)VIDEO_BUFFER)[12] = is_pressed(K_KEY) ? 'K' : 'k';
    ((char*)VIDEO_BUFFER)[14] = is_pressed(L_KEY) ? 'L' : 'l';
    ((char*)VIDEO_BUFFER)[16] = is_pressed(SEMICOLON_KEY) ? ':' : ';';
  }
}
