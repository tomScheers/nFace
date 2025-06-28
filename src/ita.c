#include "ita.h"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int get_block_brightness(BMP_image *img, size_t startX, size_t startY,
                              size_t blockWidth, size_t blockHeight, size_t block_increment);
static char brightness_to_ASCII(int brightness);

void render_ASCII(WINDOW *win, BMP_image *img, size_t ASCII_width, size_t ASCII_height, size_t block_increment) {
  size_t max_height, max_width;
  getmaxyx(win, max_height, max_width);
  size_t y_offset = (max_height - ASCII_height) / 2;
  size_t x_offset = (max_width - ASCII_width) / 2;

  size_t block_width = img->width / ASCII_width;
  size_t block_height = img->height / ASCII_height;

  if (block_width == 0)
    block_width = 1;
  if (block_height == 0)
    block_height = 1;

  for (size_t y = 0; y <= ASCII_height; ++y) {
    for (size_t x = 0; x <= ASCII_width; ++x) {
      int brightness = get_block_brightness(img, block_width * x, block_height * y,
                                          block_width, block_height, block_increment);

      mvwaddch(win, ASCII_height - y + y_offset, ASCII_width - x + x_offset,
               brightness_to_ASCII(brightness));
    }
  }
}

static int get_block_brightness(BMP_image *img, size_t start_x, size_t start_y,
                              size_t block_width, size_t block_height, size_t block_increment) {
  float total_brightness = 0;
  size_t count = 0;

  for (size_t y = start_y; y < start_y + block_height; y += block_increment) {
    for (size_t x = start_x; x < start_x + block_width; x += block_increment) {
      int index = (y * img->width + x) * 3;
      unsigned char b = img->data[index];
      unsigned char g = img->data[index + 1];
      unsigned char r = img->data[index + 2];

      float brightness = (0.2126 * r) + (0.7152 * g) + (0.0722 * b);
      total_brightness += brightness;
      ++count;
    }
  }

  if (count == 0)
    return 0;

  return (int)total_brightness / count;
}

static char brightness_to_ASCII(int brightness) {
  static const char ASCII_chars[] =
      "@$#&%WXH8Oo*+=~-:. "; // High contrast, reduced mid-tones


  size_t index = (brightness * (sizeof(ASCII_chars) - 2)) / 255;
  return ASCII_chars[sizeof(ASCII_chars) - 2 - index];

}
