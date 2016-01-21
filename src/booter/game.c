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
  Game->timer++;

  if (Game->timer % 20 == 0) {
    // visible time step
  }

  /*********************************
   * UPDATE PADDLES ON KEY PRESSES *
   *********************************/
  // Check to make sure paddle stays on screen
  // 20 horizontal paddle positions
  if (is_pressed(A_KEY))     {
    if (Game->left->paddlePos > 0.5) {
      Game->left->paddlePos -= 0.05;
    }
  }

  else if (is_pressed(D_KEY)) {
    if (Game->left->paddlePos < 0.95) {
      Game->left->paddlePos += 0.05;
    }
  }

  if (is_pressed(J_KEY))     {
    if (Game->right->paddlePos > 0.5) {
      Game->right->paddlePos -= 0.05;
    }
  }

  else if (is_pressed(L_KEY)) {
    if (Game->right->paddlePos < 0.95) {
      Game->right->paddlePos += 0.05;
    }
  }

  /**********************************
   * UPDATE BALLS FALLING WITH TIME *
   **********************************/
  Balls* currLeftBalls = Game->left->balls;
  // Move Balls down a little
  while (currLeftBalls != NULL)  {
    currLeftBalls->y += 0.01 * Game->difficulty;
    // Go to next Ball
    currLeftBalls = currLeftBalls->next;
  }

  Balls* currRightBalls = Game->right->balls;
  // Move Balls down a little
  while (currRightBalls != NULL)  {
    currRightBalls->y += 0.01 * Game->difficulty;
    // Go to next Ball
    currRightBalls = currRightBalls->next;
  }

  /************************
   * CHECK IF CAUGHT BALL *
   ************************/


}
