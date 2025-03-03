#include "camera.h"
#include "ita.h"
#include <assert.h>
#include <fcntl.h>
#include <memory.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main() {
  size_t img_width = 320, img_height = 280;
  size_t blockWidthToHeightRatio =
      2; // The ratio of a chars width in the terminal to it's height
  int factor = 128;
  float ratio = (float)((float)img_width / (float)img_height);
  int renderWidth = factor * ratio;
  int renderHeight = factor / blockWidthToHeightRatio;
  assert(renderWidth <= img_width && renderHeight <= img_height);

  initscr();
  noecho();
  cbreak();
  curs_set(0);
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);

  int startY, startX, rows, cols;
  getmaxyx(stdscr, rows, cols);

  startY = rows / 2 - renderHeight / 2;
  startX = cols / 2 - renderWidth / 2;

  printw("Terminal size: %d x %d\n", rows, cols);
  printw("Window position: %d x %d\n", startY, startX);
  printw("Image frame size: %d x %d\n", img_height, img_width);

  refresh(); // Refresh to show the debug output
  WINDOW *img_frame = newwin(renderHeight, renderWidth, startY, startX);
  box(img_frame, 0, 0);
  wrefresh(img_frame);

  int ch;
  while ((ch = getch()) != 'q') {
    captureFrame(img_width, img_height);
    BMPImg *img = loadBMP("output/output.bmp");
    renderASCII(img_frame, img, renderWidth, renderHeight);
    free(img);
    wrefresh(img_frame);
    wclear(img_frame);
    napms(50);
  }
  delwin(img_frame);
  endwin();
  return 0;
}
