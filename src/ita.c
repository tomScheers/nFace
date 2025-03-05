#include "ita.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ncurses.h>


int getBlockBrightness(BMPImg *img, int startX, int startY, int blockWidth,
                       int blockHeight, struct RGB* avColor) {
  int totalBrightness = 0;
  size_t count = 0;

  for (int y = startY; y < startY + blockHeight; ++y) {
    for (int x = startX; x < startX + blockWidth; ++x) {
      int index = (y * img->width + x) * 3;
      unsigned char b = img->data[index];
      unsigned char g = img->data[index + 1];
      unsigned char r = img->data[index + 2];

      avColor->r += r;
      avColor->g += g;
      avColor->b += b;

      int brightness = (0.2126 * r) + (0.7152 * g) + (0.0722 * b);
      totalBrightness += brightness;
      ++count;
    }
  }
  if (count == 0) return 0;
  avColor->r /= count;
  avColor->g /= count;
  avColor->b /= count;
  return totalBrightness / count;
}

char brightnessToASCII(int brightness) {
  const char asciiChars[10] = "@%#*+=-:.,";
  int index = (brightness * (sizeof(asciiChars) - 1)) / 255;
  return asciiChars[9 - index];
}

void renderASCII(WINDOW* win, BMPImg *img, int ASCIIWidth, int ASCIIHeight) {
  int blockWidth = img->width / ASCIIWidth;
  int blockHeight = img->height / ASCIIHeight;

  size_t colorPairIndex = 1;
  // Iterate from top to bottom and left to right
  for (int y = 0; y < ASCIIHeight; ++y) {
    for (int x = 0; x < ASCIIWidth; ++x) {
      // Calculate brightness for the block
      struct RGB color;
      color.b = 0;
      color.r = 0;
      color.g = 0;
      int brightness = getBlockBrightness(img, blockWidth * x, blockHeight * y,
                                          blockWidth, blockHeight, &color);

      init_color(COLOR_RED, color.r * 4, color.b * 4, color.g * 4);
      init_pair(colorPairIndex, COLOR_RED, -1);
      attron(COLOR_PAIR(colorPairIndex));
      

      // Print the corresponding ASCII character at (y, x)
      mvwprintw(win, ASCIIHeight - y, ASCIIWidth - x, "%c", brightnessToASCII(brightness));
    }
  }
}
