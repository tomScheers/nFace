#ifndef ITA_H
#define ITA_H

#include <ncurses.h>

typedef struct {
  unsigned char header[54];
  size_t width, height;
  unsigned char *data;
} BMP_image;

void render_ASCII(WINDOW *win, BMP_image *img, size_t ASCII_width, size_t ASCII_height, size_t block_increment);

#endif
