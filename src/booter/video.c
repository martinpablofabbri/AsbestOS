#include "video.h"

/* This is the address of the VGA text-mode video buffer.  Note that this
 * buffer actually holds 8 pages of text, but only the first page (page 0)
 * will be displayed.
 *
 * Individual characters in text-mode VGA are represented as two adjacent
 * bytes:
 *     Byte 0 = the character value
 *     Byte 1 = the color of the character:  the high nibble is the background
 *              color, and the low nibble is the foreground color
 *
 * See http://wiki.osdev.org/Printing_to_Screen for more details.
 *
 * Also, if you decide to use a graphical video mode, the active video buffer
 * may reside at another address, and the data will definitely be in another
 * format.  It's a complicated topic.  If you are really intent on learning
 * more about this topic, go to http://wiki.osdev.org/Main_Page and look at
 * the VGA links in the "Video" section.
 */
#define VIDEO_BUFFER ((unsigned char *) 0xB8000)
#define SET_PIX(x,y,c,f,b) do { \
  VIDEO_BUFFER[2*((y)*WIDTH+(x))] = (c); \
  VIDEO_BUFFER[2*((y)*WIDTH+(x))+1] = ((b)<<4) | (f); \
  } while (0)

void clear() {
  unsigned size = WIDTH * HEIGHT * 2;
  unsigned i = 0;
  while (i<size) {VIDEO_BUFFER[i++]=0;}
}

void init_video(void) {
  clear();
}

void draw_border () {
  // Top
  unsigned i;
  for (i=0; i<WIDTH; i++) {
    SET_PIX(i,0,'-',CYAN,RED);
    SET_PIX(i,HEIGHT-1,'-',CYAN,RED);
  }
  for (i=1; i<HEIGHT-1; i++) {
    SET_PIX(0,i,'|',CYAN,RED);
    SET_PIX(WIDTH-1,i,'|',CYAN,RED);
    SET_PIX(WIDTH/2,i,'|',CYAN,RED);
  }
}

void draw_paddles(GameState* gs) {
  // Left side
  float pos = gs->left.paddle_pos;
  float width = gs->left.paddle_width;
  float left = pos - width / 2;
  float right = pos + width / 2;
  left = (left < 0) ? 0 : left;
  right = (right > 1) ? 1 : right;
  unsigned left_idx = 1 + (int)((WIDTH / 2 - 1)*left);
  unsigned right_idx = (int)((WIDTH / 2 - 1)*right);
  int i;
  for (i=left_idx; i<=right_idx; i++) {
    SET_PIX(i, HEIGHT-2, '=',GREEN,BLACK);
  }

  // Right side
  pos = gs->right.paddle_pos;
  width = gs->right.paddle_width;
  left = pos - width / 2;
  right = pos + width / 2;
  left = (left < 0) ? 0 : left;
  right = (right > 1) ? 1 : right;
  left_idx = 1 + WIDTH/2 + (int)((WIDTH / 2 - 1)*left);
  right_idx = WIDTH/2 + (int)((WIDTH / 2 - 1)*right);
  for (i=left_idx; i<=right_idx; i++) {
    SET_PIX(i, HEIGHT-2, '=',GREEN,BLACK);
  }
}

void draw_ball (Ball* ball, unsigned left_offset) {
  // Draw a single ball
  unsigned x_idx = 1 + left_offset + \
    (int)((WIDTH / 2 - 1)*ball->x);
  unsigned y_idx = 1 + (int)((HEIGHT-1)*ball->y);
  SET_PIX(x_idx, y_idx, '@', BLUE, BLACK);
}

void draw_balls (ball_list* balls, unsigned left_offset) {
  ball_list* cur = balls;
  while (cur) {
    draw_ball(&cur->ball, left_offset);
    cur = cur->next;
  }
}

void display (GameState* gs) {
  // Draw border
  clear();
  draw_paddles(gs);
  draw_balls(gs->left.balls, 0);
  draw_balls(gs->right.balls, WIDTH/2);
  draw_border();
}
