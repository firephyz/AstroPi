#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 320
#define BYTES_PER_PIXEL 2

FILE * fb;
uint16_t * vbuf;

int main() {

  fb = fopen("/dev/fb1", "w+");
  if(fb == NULL) {
    printf("Error opening framebuffer.\n");
    exit(-1);
  }
  
  vbuf = calloc(BYTES_PER_PIXEL * SCREEN_WIDTH * \
		SCREEN_HEIGHT, BYTES_PER_PIXEL);

  int index = 0;
  for(int y = 0; y < SCREEN_HEIGHT; ++y) {
    for(int x = 0; x < SCREEN_WIDTH; ++x)  {
      if(x >= 189 && x < 289 &&
	 y >= 109 && y < 209) {
	vbuf[index] = 0xFFFF;
      }
      else {
	vbuf[index] = 0x0000;
      }

      ++index;
    }
  }

  fwrite(vbuf, BYTES_PER_PIXEL, SCREEN_WIDTH * SCREEN_HEIGHT, fb);
  
  return 0;
}
