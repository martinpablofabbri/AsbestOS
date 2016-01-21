#ifndef SRC_BOOTER_GAME_DATA_H
#define SRC_BOOTER_GAME_DATA_H

#define START_HEALTH 100
#define START_DIFFICULTY 1

// Ball, holds position and type
typedef struct {
  // Horizontal position of ball, in range [0=leftmost, 1=rightmost]
  float x;
  // Vertical position of ball, in range [0=upmost, 1=downmost]
  float y;
  // Ball type, if we implement different types of balls
  int ball_type;
} Ball;

typedef struct ball_node{
  Ball *ball;
  struct ball_node *prev;
  struct ball_node *next;
} ball_list;

typedef struct { // Halfboard
  // Horizontal paddle position, in range [0=leftmost, 1=rightmost]
  float paddle_pos;

  // Doubly-linked list of balls (has ball, prev, next fields)
  ball_list *balls;

  // Score increases as balls are catched
  int score;

  // Health decreases as balls are missed. 0=dead.
  int health;
} Halfboard;

typedef struct { // GameState
  Halfboard left, right;

  // Higher = harder
  int difficulty;
  int timer;
} GameState;

void reset_game_state(GameState* gs);

#endif  // SRC_BOOTER_GAME_DATA_H
