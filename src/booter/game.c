#include "interrupts.h"
#include "keyboard.h"
#include "video.h"
#include "game_data.h"
#include "rand.h"
#include <limits.h>

/* This is the entry-point for the game! */
void c_start(void) {
  seed(0);
  init_interrupts();
  init_timer();
  init_keyboard();
  enable_interrupts();

  /* Loop forever, so that we don't fall back into the bootloader code. */
  while (1) {
    get_keyboard_input();
    // update_game(game);
    sleep_cs(100);
  }
}

void update_game(game_state game) {

  game.timer++;

  if (game.timer % 500 == 0) {
    // Make harder every five seconds by speeding up balls
    // (harder levels have faster-falling balls,
    //  but you see less balls at a given time slice)
    game.difficulty++;
  }

  if (game.timer % 20 == 0) {
    // Add a new ball at a rate of 5 / second

    // Left Ball
    Ball newLeftBall;
    // TODO: Did I do this right?
    newLeftBall.x = float(rand()) / float(UINT_MAX);
    newLeftBall.y = 0.0;
    newLeftBall.ball_type = 0;

    ball_list *newLeftBallEntry = (ball_list*) malloc(sizeof(ball_list*));
    newLeftBallEntry->ball = newLeftBall;
    newLeftBallEntry->next = game.left.balls;
    newLeftBallEntry->prev = game.left.balls->prev;
    game.left.balls->prev = newLeftBallEntry;
    game.left.balls = newLeftBallEntry;

    // Right Ball
    Ball newRightBall;
    newRightBall.x = float(rand()) / float(UINT_MAX);
    newRightBall.y = 0.0;
    newRightBall.ball_type = 0;

    ball_list *newRightBallEntry = (ball_list*) malloc(sizeof(ball_list*));
    newRightBallEntry->ball = newRightBall;
    newRightBallEntry->next = game.right.balls;
    newRightBallEntry->prev = game.right.balls->prev;
    game.right.balls->prev = newRightBallEntry;
    game.right.balls = newRightBallEntry;
  }

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
  if (lastLeftBall->ball->y > 1) {
    if (game.left.paddle_pos + 0.05 > lastLeftBall->ball.x) {
      if (game.left.paddle_pos - 0.05 < lastLeftBall->ball.x) {
        // paddle hit ball
        game.left.score++;
        lastLeftBall->prev.next == NULL;
        // TODO:free(lastLeftBall.ball); ??
        free(lastLeftBall);
      } else {
        // paddle didn't hit ball
        game.left.health--;
        lastLeftBall->prev.next == NULL;
        // TODO:free(lastLeftBall.ball); ??
        free(lastLeftBall);
      }
    } else {
      // paddle didn't hit ball
      game.left.health--;
      lastLeftBall->prev.next == NULL;
      // TODO:free(lastLeftBall.ball); ??
      free(lastLeftBall);
    }
  }

  ball_list* lastRightBall = game.right.balls->prev;
  if (lastRightBall->ball->y > 1) {
    if (game.right.paddle_pos + 0.05 > lastRightBall->ball.x) {
      if (game.right.paddle_pos - 0.05 < lastRightBall->ball.x) {
        // paddle hit ball
        game.right.score++;
        lastRightBall->prev.next == NULL;
        // TODO:free(lastRightBall.ball); ??
        free(lastRightBall);
      } else {
        // paddle didn't hit ball
        game.right.health--;
        lastRightBall->prev.next == NULL;
        // TODO:free(lastRightBall.ball); ??
        free(lastRightBall);
      }
    } else {
      // paddle didn't hit ball
      game.right.health--;
      lastRightBall->prev.next == NULL;
      // TODO:free(lastRightBall.ball); ??
      free(lastRightBall);
    }
  }
}
