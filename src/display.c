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

int in_display_bounds(DisplayData * display, int x, int y) {
  return x >= 0 && x < display->width && y >= 0 && y < display->height;
}

void display_draw_circle(DisplayData * display, int x_pos, int y_pos, int radius) {

  int r2 = radius * radius;
  
  int x = 0;
  int y = radius;

  if(in_display_bounds(display, x_pos, y_pos)) {
    display->buffer[x_pos + y_pos * display->width] = display->color;
  }

  while(y >= x) {
    if(x * x + y * y <= r2) {
      for(int i = 0; i < 8; ++i) {
	int pix_x = x_pos;
	int pix_y = y_pos;
	switch(i) {
	case 0:
	  pix_x += x;
	  pix_y += y;
	  break;
	case 1:
	  pix_x -= x;
	  pix_y += y;
	  break;
	case 2:
	  pix_x += x;
	  pix_y -= y;
	  break;
	case 3:
	  pix_x -= x;
	  pix_y -= y;
	  break;
	case 4:
	  pix_x += y;
	  pix_y += x;
	  break;
	case 5:
	  pix_x -= y;
	  pix_y += x;
	  break;
	case 6:
	  pix_x += y;
	  pix_y -= x;
	  break;
	case 7:
	  pix_x -= y;
	  pix_y -= x;
	  break;
	}
	if(in_display_bounds(display, pix_x, pix_y)) {
	  display->buffer[pix_x + pix_y * display->width] = display->color;
	}
      }
      x += 1;
    }
    else {
      y -= 1;
    }
  }
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
