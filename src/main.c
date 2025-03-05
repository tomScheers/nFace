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

  init_pair(1, COLOR_BLACK, COLOR_BLACK);

  bkgd(COLOR_PAIR(1));
}

int main() {
  size_t imageWidth = 640, imageHeight = 360;

  init_ncurses();

  startCamera(imageWidth, imageHeight);

  return 0;
}
