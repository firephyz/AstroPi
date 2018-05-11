#ifndef FONTS_INCLUDED

#define FONTS_INCLUDED

#define FONT_WIDTH 6
#define FONT_HEIGHT 8

#include "display.h"

typedef struct {
  int bits[FONT_WIDTH * FONT_HEIGHT];
} Glyph;

extern Glyph * font_glyphs;

void drawString(DisplayData * window, int x, int y, char * string);
void drawChar(DisplayData * window, int x, int y, char c);
void drawFontTable(DisplayData * display, int x, int y);
void initFontGlyphs();

#endif
