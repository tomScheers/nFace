#include "camera.h"
#include <assert.h>
#include <fcntl.h>
#include <memory.h>
#include <ncurses.h>
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

  init_color(COLOR_RED, 1000, 0, 0);
  init_color(COLOR_GREEN, 0, 1000, 0);
  init_color(COLOR_BLUE, 0, 0, 1000);

  init_pair(1, COLOR_RED, COLOR_BLACK);
  init_pair(2, COLOR_GREEN, COLOR_BLACK);
  init_pair(3, COLOR_BLUE, COLOR_BLACK);
  init_pair(4, COLOR_YELLOW, COLOR_BLACK);
  init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(6, COLOR_WHITE, COLOR_BLACK);
  init_pair(7, COLOR_BLACK, COLOR_BLACK);
}

int main() {
  size_t imageWidth = 640, imageHeight = 460;
  //float blockWidthToHeightRatio =
  //    2.0; // The ratio of a chars width in the terminal to it's height
  //int factor = 200;
  //float ratio = (float)imageWidth / (float)imageHeight;
  //size_t renderWidth = factor * ratio;
  //size_t renderHeight = factor / blockWidthToHeightRatio;
  //assert(renderWidth <= imageWidth && renderHeight <= imageHeight);

  init_ncurses();

  startCamera(imageWidth, imageHeight);

  return 0;
}
