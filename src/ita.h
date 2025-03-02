#ifndef ITA_H
#define ITA_H

#include <stdbool.h>

typedef struct {
  unsigned char header[54];
  int width, height;
  unsigned char *data;
} BMPImg;

BMPImg *loadBMP(const char *fileName);
int getBlockBrightness(BMPImg *img, int startX, int startY, int blockWidth, int blockHeight);
char brightnessToASCII(int brightness);
void renderASCII(BMPImg *img, int asciiWidth, int asciiHeight);

#endif
