#include "camera.h"
#include <assert.h>
#include <fcntl.h>
#include <memory.h>
#include <ncurses.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#define PROJECT_VERSION "1.0.1"
#define HEIGHT_WIDTH_RATIO 2.1.1

static void print_help();
static void init_ncurses();
WINDOW *createWindow(size_t imageWidth, size_t imageHeight);
static int
xstreq(const char *str1,
       const char *str2); // Helper function, checks if two strs are equal

int main(int argc, char *argv[]) {
  size_t imageWidth = 640, imageHeight = 360;
  size_t frameWidth = 0, frameHeight = 0;

  for (int i = 1; i < argc; i += 2) {
    if (xstreq(argv[i], "-w") || xstreq(argv[i], "--width")) {
      if (argc <= i + 1) {
        printf("The %s flag requires an integer argument to be passed in.\n", argv[i]);
        return EXIT_FAILURE;
      }
      frameWidth = (size_t)atoi(argv[i + 1]) - 1;
    } else if (xstreq(argv[i], "-h") || xstreq(argv[i], "--height")) {
      if (argc <= i + 1) {
        printf("The %s flag requires an integer argument to be passed in.\n", argv[i]);
        return EXIT_FAILURE;
      }
      frameHeight = (size_t)atoi(argv[i + 1]) - 1;
    } else if (xstreq(argv[i], "--version") || xstreq(argv[i], "-v")) {
      printf("nFace version: %s\n", PROJECT_VERSION);
      return EXIT_SUCCESS;
    } else {
      print_help();
      return EXIT_SUCCESS;
    }
  }

  init_ncurses();

  WINDOW *imageFrame = createWindow(imageWidth, imageHeight);
  int cameraFd = openCamera();
  if (cameraFd == -1) {
    perror("Opening camera");
    goto error;
  }

  struct v4l2_format fmt = {0};
  if (setFormat(cameraFd, imageWidth, imageHeight, &fmt) == -1) {
    perror("Setting format");
    goto error;
  }

  if (requestBuffer(cameraFd) == -1) {
    perror("Requesting buffer");
    goto error;
  }

  struct v4l2_buffer buf;
  if (getDeviceInfo(cameraFd, &buf) == -1) {
    perror("Fetching device info");
    goto error;
  }

  unsigned char *yuyv = mapMemory(cameraFd, &buf);
  if (yuyv == MAP_FAILED) {
    perror("Mapping memory");
    goto error;
  }

  if (startStream(cameraFd) == -1) {
    perror("Start streaming");
    munmap(yuyv, buf.length);
    goto error;
  }

  size_t max_height, max_width;
  getmaxyx(imageFrame, max_height, max_width);

  if (frameHeight == 0 || frameHeight > max_height)
    frameHeight = max_height;

  if (frameWidth == 0 || frameWidth > max_width)
    frameWidth = max_width;

  while (getch() != 'q') {
    yuyv = mapMemory(cameraFd, &buf);
    if (yuyv == MAP_FAILED) {
      perror("Mapping memory");
      goto error;
    }

    if (queueBuffer(cameraFd) == -1) {
      perror("Queueing buffer");
      munmap(yuyv, buf.length);
      goto error;
    }

    if (selectFrame(cameraFd) == -1) {
      perror("Selecting frame");
      munmap(yuyv, buf.length);
      goto error;
    }

    if (dequeueBuf(cameraFd, &buf) == -1) {
      perror("Dequeueing buffer");
      munmap(yuyv, buf.length);
      goto error;
    }

    BMPImage *image = calloc(1, sizeof(BMPImage));
    if (image == NULL) {
      perror("malloc");
      munmap(yuyv, buf.length);
      free(image->data);
      free(image);
      goto error;
    }

    unsigned char *infoHeader = getImageHeader(imageWidth, imageHeight);
    memcpy(&image->header, infoHeader, sizeof(*infoHeader));

    image->width = imageWidth;
    image->height = imageHeight;

    size_t dataSize = imageWidth * imageHeight * 3;

    writeImageData(image, yuyv, dataSize, fmt.fmt.pix.bytesperline);

    if (image->data == NULL) {
      perror("malloc");
      munmap(yuyv, buf.length);
      free(image);
      free(infoHeader);
      goto error;
    }

    werase(imageFrame);

    renderASCII(imageFrame, image, frameWidth, frameHeight);

    wrefresh(imageFrame);

    munmap(yuyv, buf.length);
    free(infoHeader);
    free(image->data);
    free(image);
  }

  if (endStream(cameraFd) == -1) {
    perror("Ending stream");
    close(cameraFd);
    delwin(imageFrame);
    endwin();
    return EXIT_FAILURE;
  }

  close(cameraFd);
  delwin(imageFrame);
  endwin();
  return EXIT_SUCCESS;

error:
  if (endStream(cameraFd) == -1)
    perror("Ending stream");

  close(cameraFd);
  delwin(imageFrame);
  endwin();
  return EXIT_FAILURE;
}

static void init_ncurses() {
  initscr();
  noecho();
  cbreak();
  curs_set(0);
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);
  start_color();
  init_pair(1, COLOR_WHITE, COLOR_BLACK);
  bkgd(COLOR_PAIR(1));
}

WINDOW *createWindow(size_t imageWidth, size_t imageHeight) {
  int termRows, termCols;
  getmaxyx(stdscr, termRows, termCols);
  size_t stTermRows = (size_t)termRows;
  size_t stTermCols = (size_t)termCols;

  size_t scaledImageHeight = imageHeight / HEIGHT_WIDTH_RATIO;

  int frameWidth = (imageWidth <= stTermCols) ? imageWidth : stTermCols;
  int frameHeight =
      (scaledImageHeight <= stTermRows) ? scaledImageHeight : stTermRows;

  // Centering calculations
  int startY = (stTermRows - frameHeight) / 2;
  int startX = (stTermCols - frameWidth) / 2;

  return newwin(frameHeight, frameWidth, startY, startX);
}

static int xstreq(const char *str1, const char *str2) {
  return strcmp(str1, str2) == 0;
}

static void print_help() {
  printf("Usage: nface [OPTION]\n");
  printf("A simple camera interface through the terminal\n\n");
  printf("Options:\n");
  printf("  --help    Display this help message and exit.\n");
  printf("  -v, --version   Show version information and exit.\n");
  printf("  -w, --width Change the width of the camera frame.\n");
  printf("  -h, --height  Change the height of the camera frame.\n");
}
