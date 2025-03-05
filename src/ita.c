#include "ita.h"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int getBlockBrightness(BMPImg *img, int startX, int startY, int blockWidth,
                       int blockHeight, struct RGB *avColor) {
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
  if (count == 0)
    return 0;
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

int getClosestColor(int r, int g, int b) {
    const struct RGB ncursesColors[] = {
        {255, 0, 0},     // COLOR_RED
        {0, 255, 0},     // COLOR_GREEN
        {0, 0, 255},     // COLOR_BLUE
        {255, 255, 0},   // COLOR_YELLOW
        {255, 0, 255},   // COLOR_MAGENTA
        {255, 255, 255}, // COLOR_WHITE
        {0, 0, 0},       // COLOR_BLACK
    };

    const int NCURSES_COLOR_COUNT = sizeof(ncursesColors) / sizeof(ncursesColors[0]);
    int colorReturn = 0;
    int smallestDistanceSquared = 1000 * 1000; // a value bigger than any possible squared distance
    for (int i = 0; i < NCURSES_COLOR_COUNT; i++) {
        int dr = r - ncursesColors[i].r;
        int dg = g - ncursesColors[i].g;
        int db = b - ncursesColors[i].b;
        int distanceSquared = dr * dr + dg * dg + db * db;
        if (distanceSquared < smallestDistanceSquared) {
            smallestDistanceSquared = distanceSquared;
            colorReturn = i;
        }
    }
    return colorReturn;
}

void renderASCII(WINDOW *win, BMPImg *img, int ASCIIWidth, int ASCIIHeight) {
  int blockWidth = img->width / ASCIIWidth;
  int blockHeight = img->height / ASCIIHeight;

  if (blockWidth == 0)
    blockWidth = 1;
  if (blockHeight == 0)
    blockHeight = 1;

  for (int y = 0; y < ASCIIHeight; ++y) {
    for (int x = 0; x < ASCIIWidth; ++x) {
      struct RGB color;
      int brightness = getBlockBrightness(img, blockWidth * x, blockHeight * y,
                                          blockWidth, blockHeight, &color);

      //int closestColor = getClosestColor(color.r, color.g, color.b);
      //wattron(win, COLOR_PAIR(closestColor));

      // Corrected indexing
      mvwprintw(win, ASCIIHeight - y, ASCIIWidth - x, "%c",
                brightnessToASCII(brightness));

//wattroff(win, COLOR_PAIR(closestColor));
    }
  }
}
