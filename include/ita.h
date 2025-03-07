#ifndef ITA_H
#define ITA_H

#include <stdbool.h>
#include <ncurses.h>

typedef struct {
  unsigned char header[54];
  int width, height;
  unsigned char *data;
} BMPImg;

//int getBlockBrightness(BMPImg *img, int startX, int startY, int blockWidth, int blockHeight, struct RGB *avColor);
//char brightnessToASCII(int brightness);
void renderASCII(WINDOW* win, BMPImg *img, int asciiWidth, int asciiHeight);

#endif
