#include "ita.h"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int getBlockBrightness(BMPImage *img, int startX, int startY, int blockWidth, int blockHeight);
static char brightnessToASCII(int brightness);

void renderASCII(WINDOW *win, BMPImage *img, int ASCIIWidth, int ASCIIHeight) {
  int blockWidth = img->width / ASCIIWidth;
  int blockHeight = img->height / ASCIIHeight;

  if (blockWidth == 0)
    blockWidth = 1;
  if (blockHeight == 0)
    blockHeight = 1;

  for (int y = 0; y < ASCIIHeight; ++y) {
    for (int x = 0; x < ASCIIWidth; ++x) {
      int brightness = getBlockBrightness(img, blockWidth * x, blockHeight * y,
                                          blockWidth, blockHeight);

      mvwprintw(win, ASCIIHeight - y, ASCIIWidth - x, "%c",
                brightnessToASCII(brightness));
    }
  }
}

static int getBlockBrightness(BMPImage *img, int startX, int startY, int blockWidth,
                       int blockHeight) {
  int totalBrightness = 0;
  size_t count = 0;

  size_t blockIncrement = 1; // The higher the value, the more efficient the program, but the less accurate the image is

  for (int y = startY; y < startY + blockHeight; y += blockIncrement) {
    for (int x = startX; x < startX + blockWidth; x += blockIncrement) {
      int index = (y * img->width + x) * 3;
      unsigned char b = img->data[index];
      unsigned char g = img->data[index + 1];
      unsigned char r = img->data[index + 2];

      int brightness = (0.2126 * r) + (0.7152 * g) + (0.0722 * b);
      totalBrightness += brightness;
      ++count;
    }
  }

  if (count == 0)
    return 0;

  return totalBrightness / count;
}

static char brightnessToASCII(int brightness) {
  const char asciiChars[] = "@$#&%WXH8Oo*+=~-:. "; // High contrast, reduced mid-tones
  int index = (brightness * (sizeof(asciiChars) - 2)) / 255;
  return asciiChars[sizeof(asciiChars) - 2 - index];
}
