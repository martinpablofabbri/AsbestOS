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

void update_game(game_state game) {

/* TODO: - UPDATE DIFFICULTY & NEW BALLS WITH TIME
  game.timer++;

  if (game.timer % 500 == 0) {
    // visible time step
    game.difficulty++;
  }

  if (game.timer % 20 == 0) {
    // TODO: Add new ball
  }
*/

  /*********************************
   * UPDATE PADDLES ON KEY PRESSES *
   *********************************/
  // Check to make sure paddle stays on screen
  // 20 horizontal paddle positions
  if (is_pressed(A_KEY))     {
    if (game.left.paddle_pos > 0.05) {
      game.left.paddle_pos -= 0.05;
    }
  }

  else if (is_pressed(D_KEY)) {
    if (game.left.paddle_pos < 0.95) {
      game.left.paddle_pos += 0.05;
    }
  }

  if (is_pressed(J_KEY))     {
    if (game.right.paddle_pos > 0.5) {
      game.right.paddle_pos -= 0.05;
    }
  }

  else if (is_pressed(L_KEY)) {
    if (game.right.paddle_pos < 0.95) {
      game.right.paddle_pos += 0.05;
    }
  }

  /**********************************
   * UPDATE BALLS FALLING WITH TIME *
   **********************************/
  ball_list* currLeftBalls = game.left.balls;
  // Move Balls down a little
  while (currLeftBalls != NULL)  {
    currLeftBalls->ball.y += 0.01 * game.difficulty;
    // Go to next Ball
    currLeftBalls = currLeftBalls->next;
  }

  ball_list* currRightBalls->next = game.right.balls;
  // Move Balls down a little
  while (currRightBalls != NULL)  {
    currRightBalls->ball.y += 0.01 * game.difficulty;
    // Go to next Ball
    currRightBalls = currRightBalls->next;
  }

  /************************
   * CHECK IF CAUGHT BALL *
   ************************/
  ball_list* lastLeftBall = game.left.balls->prev;
  if (lastLeftBall->ball->y < 1) {
    if (game.left.paddle_pos + 0.05 > lastLeftBall->ball.x) {
      if (game.left.paddle_pos - 0.05 < lastLeftBall->ball.x) {
        // paddle hit ball
        game.left.score++;
        lastLeftBall->prev == NULL;
        // TODO: free lastLeftBall;
      } else {
        // paddle didn't hit ball
        game.left.health--;
        lastLeftBall->prev == NULL;
        // TODO: free lastLeftBall;
      }
    } else {
      // paddle didn't hit ball
      game.left.health--;
      lastLeftBall->prev == NULL;
      // TODO: free lastLeftBall;
    }
  }

  ball_list* lastRightBall = game.right.balls->prev;
  if (lastRightBall->ball->y < 1) {
    if (game.right.paddle_pos + 0.05 > lastRightBall->ball.x) {
      if (game.right.paddle_pos - 0.05 < lastRightBall->ball.x) {
        // paddle hit ball
        game.right.score++;
        lastRightBall->prev == NULL;
        // TODO: free lastRightBall;
      } else {
        // paddle didn't hit ball
        game.right.health--;
        lastRightBall->prev == NULL;
        // TODO: free lastRightBall;
      }
    } else {
      // paddle didn't hit ball
      game.right.health--;
      lastRightBall->prev == NULL;
      // TODO: free lastRightBall;
    }
  }

}
