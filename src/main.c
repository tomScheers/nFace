#include "camera.h"
#include <assert.h>
#include <fcntl.h>
#include <memory.h>
#include <ncurses.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#define PROJECT_VERSION "1.0.0"

static void init_ncurses();
WINDOW *createWindow(size_t imageWidth, size_t imageHeight);

int main(int argc, char *argv[]) {
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0) {
      printf("nFace version: %s\n", PROJECT_VERSION);
      return EXIT_SUCCESS;
    }
    if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      printf("Usage: nface [OPTION]\n");
      printf("A simple camera interface through the terminal\n\n");
      printf("Options:\n");
      printf("  -h, --help    Display this help message and exit.\n");
      printf("  -v, --version   Show version information and exit.\n");
      return EXIT_SUCCESS;
    }
  }
  size_t imageWidth = 640, imageHeight = 360;

  init_ncurses();

  WINDOW *imageFrame = createWindow(imageWidth, imageHeight);
  int cameraFd = openCamera();
  if (cameraFd == -1) {
    perror("Opening camera");
    return EXIT_FAILURE;
  }

  struct v4l2_format fmt = {0};
  if (setFormat(cameraFd, imageWidth, imageHeight, &fmt) == -1) {
    perror("Setting format");
    close(cameraFd);
    return EXIT_FAILURE;
  }

  if (requestBuffer(cameraFd) == -1) {
    perror("Requesting buffer");
    close(cameraFd);
    return EXIT_FAILURE;
  }

  struct v4l2_buffer buf;
  if (getDeviceInfo(cameraFd, &buf) == -1) {
    perror("Fetching device info");
    close(cameraFd);
    return EXIT_FAILURE;
  }

  unsigned char *yuyv = mapMemory(cameraFd, &buf);
  if (yuyv == MAP_FAILED) {
    perror("Mapping memory");
    close(cameraFd);
    return EXIT_FAILURE;
  }

  if (startStream(cameraFd) == -1) {
    perror("Start streaming");
    munmap(yuyv, buf.length);
    close(cameraFd);
    return EXIT_FAILURE;
  }

  size_t frameWidth, frameHeight;
  getmaxyx(imageFrame, frameHeight, frameWidth);

  while (getch() != 'q') {
    yuyv = mapMemory(cameraFd, &buf);
    if (yuyv == MAP_FAILED) {
      perror("Mapping memory");
      close(cameraFd);
      return EXIT_FAILURE;
    }

    if (queueBuffer(cameraFd) == -1) {
      perror("Queueing buffer");
      munmap(yuyv, buf.length);
      close(cameraFd);
      return EXIT_FAILURE;
    }

    if (selectFrame(cameraFd) == -1) {
      perror("Selecting frame");
      munmap(yuyv, buf.length);
      close(cameraFd);
      return EXIT_FAILURE;
    }

    if (dequeueBuf(cameraFd, &buf) == -1) {
      perror("Dequeueing buffer");
      munmap(yuyv, buf.length);
      close(cameraFd);
      return EXIT_FAILURE;
    }

    BMPImage *image = malloc(sizeof(BMPImage));

    unsigned char *infoHeader = getImageHeader(imageWidth, imageHeight);
    memcpy(&image->header, infoHeader, sizeof(*infoHeader));

    image->width = imageWidth;
    image->height = imageHeight;

    size_t dataSize = imageWidth * imageHeight * 3;

    writeImageData(image, yuyv, dataSize, fmt.fmt.pix.bytesperline);

    werase(imageFrame);

    renderASCII(imageFrame, image, frameWidth, frameHeight);
    munmap(yuyv, buf.length);
    wrefresh(imageFrame);
    free(infoHeader);
    free(image->data);
    free(image);
  }

  if (endStream(cameraFd) == -1) {
    perror("Ending stream");
    close(cameraFd);
    return EXIT_FAILURE;
  }

  close(cameraFd);
  delwin(imageFrame);
  endwin();

  return 0;
}

static void init_ncurses() {
  initscr();
  noecho();
  cbreak();
  curs_set(0);
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);
}

WINDOW *createWindow(size_t imageWidth, size_t imageHeight) {
  int termRows, termCols;
  getmaxyx(stdscr, termRows, termCols);

  size_t scaledImageHeight = imageHeight / 2.1;

  int frameWidth = (imageWidth <= termCols) ? imageWidth : termCols;
  int frameHeight =
      (scaledImageHeight <= termRows) ? scaledImageHeight : termRows;

  // Centering calculations
  int startY = (termRows - frameHeight) / 2;
  int startX = (termCols - frameWidth) / 2;

  return newwin(frameHeight, frameWidth, startY, startX);
}
