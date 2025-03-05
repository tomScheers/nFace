#include "camera.h"
#include <assert.h>
#include <fcntl.h>
#include <memory.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

static void init_ncurses() {
  initscr();
  noecho();
  cbreak();
  curs_set(0);
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);
  start_color();
  use_default_colors();
}

int main() {
  size_t imageWidth = 640, imageHeight = 560;
  size_t blockWidthToHeightRatio =
      2; // The ratio of a chars width in the terminal to it's height
  int factor = 200;
  float ratio = (float)((float)imageWidth / (float)imageHeight);
  size_t renderWidth = factor * ratio;
  size_t renderHeight = factor / blockWidthToHeightRatio;
  assert(renderWidth <= imageWidth && renderHeight <= imageHeight);

  init_ncurses();

  startCamera(imageWidth, imageHeight, renderWidth, renderHeight);

  return 0;
}
