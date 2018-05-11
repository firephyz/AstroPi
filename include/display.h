#ifndef DISPLAY_INCLUDED

#define DISPLAY_INCLUDED

#include <stdint.h>
#include <stdio.h>

#define DISPLAY_WIDTH 480
#define DISPLAY_HEIGHT 320

#define NEW_COLOR(r, g, b) (((r & 0x1F) << 11) + ((g & 0x3F) << 5) + (b & 0x1F))
#define COLOR_BLACK NEW_COLOR(0, 0, 0)
#define COLOR_WHITE NEW_COLOR(0x1F, 0x3F, 0x1F)
#define COLOR_RED NEW_COLOR(0x1F, 0, 0)
#define COLOR_GREEN NEW_COLOR(0, 0x3F, 0)
#define COLOR_BLUE NEW_COLOR(0, 0, 0x1F)
#define COLOR_GREY NEW_COLOR(0xF, 0x1F, 0xF)

typedef struct {
  FILE * fb;
  uint16_t color;
  uint16_t * buffer;
  int width;
  int height;
  int pixCount;
} DisplayData;

DisplayData * init_display();

void display_set_color(DisplayData * display, uint16_t color);
void display_clear(DisplayData * display);
void display_update(DisplayData * display);

int in_display_bounds(DisplayData * display, int x, int y);

void display_draw_circle(DisplayData * display, int x, int y, int radius);

void display_draw_rect(DisplayData * display, int x, int y, int width, int height);
void display_fill_rect(DisplayData * display, int x, int y, int width, int height);

#endif



