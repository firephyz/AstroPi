#include "display.h"
#include <string.h>

void display_set_color(DisplayData * display, uint16_t color) {
  display->color = color;
}

void display_clear(DisplayData * display) {
  for(int i = 0; i < display->pixCount; ++i) {
    display->buffer[i] = display->color;
  }
  display_update(display);
}

void display_update(DisplayData * display) {
  fwrite(display->buffer,
	 sizeof(uint16_t),
	 display->pixCount,
	 display->fb);
  rewind(display->fb);
}

void display_draw_rect(DisplayData * display, int x, int y, int width, int height) {

  // Draw top line
  int startIndex = x + y * display->width;
  for(int i = startIndex; i < startIndex + width; ++i) {
    display->buffer[i] = display->color;
  }

  // Draw left line
  int endIndex = startIndex + display->width * height;
  for(int i = startIndex; i < endIndex; i += display->width) {
    display->buffer[i] = display->color;
  }

  // Draw bottom line
  startIndex = x + (y + height - 1) * display->width;
  for(int i = startIndex; i < startIndex + width; ++i) {
    display->buffer[i] = display->color;
  }

  // Draw right line
  startIndex = x + y * display->width + width - 1;
  endIndex = startIndex + display->width * height;
  for(int i = startIndex; i < endIndex; i += display->width) {
    display->buffer[i] = display->color;
  }
}

void display_fill_rect(DisplayData * display, int x, int y, int width, int height) {

  int index = x + y * display->width;
  for(int j = 0; j < height; ++j) {
    for(int i = 0; i < width; ++i) {
      display->buffer[index] = display->color;
      index += 1;
    }
    index += display->width - width;
  }
}
