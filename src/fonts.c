#include "fonts.h"
#include "display.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void drawString(DisplayData * window, int x, int y, char * string) {
  for(int i = 0; i < strlen(string); ++i) {
    drawChar(window, x, y, string[i]);
    x += FONT_WIDTH;
  }
}

void drawChar(DisplayData * window, int x, int y, char c) {
  for(int h = 0; h < FONT_HEIGHT; ++h) {
    for(int w = 0; w < FONT_WIDTH; ++w) {
      window->buffer[x + w + (y + h) * window->width] = font_glyphs[(int)c].bits[w + FONT_WIDTH * h] * COLOR_WHITE;
    }
  }
}

void initFontGlyphs() {

  font_glyphs = malloc(sizeof(Glyph) * 128);
  FILE * font_file = fopen("font_file.txt", "r");

  for(int c = 0; c < 128; ++c) {
    int index = 0;
    char in = fgetc(font_file);
    while(in  != '\n') {
      if(in != ' ') {
	font_glyphs[c].bits[index] = in - '0';
	++index;
      }
      in = fgetc(font_file);
    }
  }

  fclose(font_file);
}
