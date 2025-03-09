#ifndef ITA_H
#define ITA_H

#include <ncurses.h>

typedef struct {
  unsigned char header[54];
  int width, height;
  unsigned char *data;
} BMPImage;

void renderASCII(WINDOW* win, BMPImage *img, int asciiWidth, int asciiHeight);

#endif
