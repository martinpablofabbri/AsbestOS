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
    SET_PIX(i,0,'-',CYAN,GREEN);
    SET_PIX(i,HEIGHT-1,'-',CYAN,GREEN);
  }
  for (i=1; i<HEIGHT-1; i++) {
    SET_PIX(0,i,'|',CYAN,GREEN);
    SET_PIX(WIDTH-1,i,'|',CYAN,GREEN);
    SET_PIX(WIDTH/2,i,'|',CYAN,GREEN);
  }
}

void display (GameState* gs) {
  // Draw border
  draw_border();
}
