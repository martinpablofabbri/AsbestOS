#include "interrupts.h"
#include "keyboard.h"
#include "video.h"
#include "game_data.h"
#include "rand.h"
#include <limits.h>

#define NUM_BALLS 30
static ball_list all_balls[NUM_BALLS];

// Makes sure all balls are free
void init_balls() {
  int i;
  for (i=0; i<NUM_BALLS; i++) {
    all_balls[i].in_use = 0;
  }
}

// Returns a pointer to a new ball_list, or 0 on failure
ball_list* new_ball() {
  int i;
  for (i=0; i<NUM_BALLS; i++) {
    if (all_balls[i].in_use == 0) {
      // This ball is free
      ball_list* b = &all_balls[i];
      b->in_use = 1;
      return b;
    }
  }
  return 0;
}

void free_ball(ball_list* b) {
  b->in_use = 0;
}

void update_game(GameState* gs);

void reset_game_state(GameState* gs) {
  init_balls();
  gs->difficulty = START_DIFFICULTY;

  gs->left.paddle_pos = 0.5;
  gs->left.paddle_width = 0.2;
  gs->left.balls = 0;
  gs->left.score = 0;
  gs->left.health = START_HEALTH;

  gs->right.paddle_pos = 0.5;
  gs->right.paddle_width = 0.2;
  gs->right.balls = 0;
  gs->right.score = 0;
  gs->right.health = START_HEALTH;

  gs->timer = 0;
}

/* This is the entry-point for the game! */
void c_start(void) {
  seed(0);
  init_interrupts();
  init_timer();
  init_keyboard();
  init_video();
  GameState s;
  reset_game_state(&s);

  enable_interrupts();

  /* Loop forever, so that we don't fall back into the bootloader code. */
  while (1) {
    get_keyboard_input();
    update_game(&s);
    display(&s);
    sleep_cs(2);
  }
}

void update_game(GameState* game) {
  game->timer++;

  if (game->timer % 500 == 0) {
    // Make harder every five seconds by speeding up balls
    // (harder levels have faster-falling balls,
    //  but you see less balls at a given time slice)
    game->difficulty++;
  }


  if (game->timer % 20 == 0) {
    // Add a new ball at a rate of 5 / second

    // Left Ball
    ball_list *newLeftBallEntry = new_ball();
    if (newLeftBallEntry) {
      Ball* newLeftBall = &newLeftBallEntry->ball;

      newLeftBallEntry->next = game->left.balls;
      if (game->left.balls) {
        newLeftBallEntry->prev = game->left.balls->prev;
        game->left.balls->prev = newLeftBallEntry;
      } else {
        newLeftBallEntry->prev = newLeftBallEntry;
      }
      game->left.balls = newLeftBallEntry;

      newLeftBall->x = (float)(rand()) / (float)(UINT_MAX);
      newLeftBall->y = 0.0;
      newLeftBall->ball_type = 0;
    }

    // Right Ball
    ball_list *newRightBallEntry = new_ball();
    if (newRightBallEntry) {
      Ball* newRightBall = &newRightBallEntry->ball;

      newRightBallEntry->next = game->right.balls;
      if (game->right.balls) {
        newRightBallEntry->prev = game->right.balls->prev;
        game->right.balls->prev = newRightBallEntry;
      } else {
        newRightBallEntry->prev = newRightBallEntry;
      }
      game->right.balls = newRightBallEntry;

      newRightBall->x = (float)(rand()) / (float)(UINT_MAX);
      newRightBall->y = 0.0;
      newRightBall->ball_type = 0;
    }
  }

  /*********************************
   * UPDATE PADDLES ON KEY PRESSES *
   *********************************/
  // Check to make sure paddle stays on screen
  // 20 horizontal paddle positions
  if (is_pressed(S_KEY)) {
    if (game->left.paddle_pos > 0.01) {
      game->left.paddle_pos -= 0.01;
    }
  }

  else if (is_pressed(A_KEY)) {
    if (game->left.paddle_pos > 0.05) {
      game->left.paddle_pos -= 0.05;
    }
  }

  else if (is_pressed(D_KEY)) {
    if (game->left.paddle_pos < 0.99) {
      game->left.paddle_pos += 0.01;
    }
  }

  else if (is_pressed(F_KEY)) {
    if (game->left.paddle_pos < 0.95) {
      game->left.paddle_pos += 0.05;
    }
  }

  if (is_pressed(K_KEY)) {
    if (game->right.paddle_pos > 0.01) {
      game->right.paddle_pos -= 0.01;
    }
  }

  else if (is_pressed(J_KEY)) {
    if (game->right.paddle_pos > 0.05) {
      game->right.paddle_pos -= 0.05;
    }
  }

  else if (is_pressed(L_KEY)) {
    if (game->right.paddle_pos < 0.99) {
      game->right.paddle_pos += 0.01;
    }
  }

  else if (is_pressed(SEMICOLON_KEY)) {
    if (game->right.paddle_pos < 0.95) {
      game->right.paddle_pos += 0.05;
    }
  }

  /**********************************
   * UPDATE BALLS FALLING WITH TIME *
   **********************************/
  ball_list* currLeftBalls = game->left.balls;
  // Move Balls down a little
  while (currLeftBalls != 0)  {
    currLeftBalls->ball.y += 0.01 * game->difficulty;
    // Go to next Ball
    currLeftBalls = currLeftBalls->next;
  }

  ball_list* currRightBalls = game->right.balls;
  // Move Balls down a little
  while (currRightBalls != 0)  {
    currRightBalls->ball.y += 0.01 * game->difficulty;
    // Go to next Ball
    currRightBalls = currRightBalls->next;
  }

  /************************
   * CHECK IF CAUGHT BALL *
   ************************/
  ball_list* lastLeftBall = game->left.balls->prev;
  if (lastLeftBall->ball.y > 1) {
    if (game->left.paddle_pos + game->left.paddle_width / 2 > lastLeftBall->ball.x) {
      if (game->left.paddle_pos - game->left.paddle_width / 2 < lastLeftBall->ball.x) {
        // paddle hit ball
        game->left.score++;
      } else {
        // paddle didn't hit ball
        game->left.health--;
      }
    } else {
      // paddle didn't hit ball
      game->left.health--;
    }
    // Remove the last left ball from the list
    if (lastLeftBall->prev == lastLeftBall) {
      // It's the last ball left
      game->left.balls = 0;
    } else {
      lastLeftBall->prev->next = 0;
      game->left.balls->prev = lastLeftBall->prev;
    }
    free_ball(lastLeftBall);
  }

  ball_list* lastRightBall = game->right.balls->prev;
  if (lastRightBall->ball.y > 1) {
    if (game->right.paddle_pos + game->right.paddle_width / 2 > lastRightBall->ball.x) {
      if (game->right.paddle_pos - game->right.paddle_width / 2 < lastRightBall->ball.x) {
        // paddle hit ball
        game->right.score++;
      } else {
        // paddle didn't hit ball
        game->right.health--;
      }
    } else {
      // paddle didn't hit ball
      game->right.health--;
    }
    // Remove the last right ball from the list
    if (lastRightBall->prev == lastRightBall) {
      // It's the last ball right
      game->right.balls = 0;
    } else {
      lastRightBall->prev->next = 0;
      game->right.balls->prev = lastRightBall->prev;
    }
    free_ball(lastRightBall);
  }

  // Check for end of game
  if (game->right.health == 0 || game->right.health == 0) {
    // Died :(
    reset_game_state(game);
  }
}
