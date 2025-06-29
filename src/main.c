#include "camera.h"
#include "version.h"
#include <assert.h>
#include <fcntl.h>
#include <memory.h>
#include <ncurses.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#define HEIGHT_WIDTH_RATIO 2.1

static void print_help();
static void init_ncurses();
WINDOW *create_window(size_t image_width, size_t image_height);
static int
xstreq(const char *str1,
       const char *str2); // Helper function, checks if two strs are equal

int main(int argc, char *argv[]) {
  size_t image_width = 640, image_height = 360;
  size_t frame_width = 0, frame_height = 0;
  size_t block_increment = 2; // Default for block increment

  for (int i = 1; i < argc; i += 2) {
    if (xstreq(argv[i], "-w") || xstreq(argv[i], "--width")) {
      if (argc <= i + 1) {
        printf("The %s flag requires an integer argument to be passed in.\n", argv[i]);
        return EXIT_FAILURE;
      }
      frame_width = (size_t)atoi(argv[i + 1]) - 1;
    } else if (xstreq(argv[i], "-h") || xstreq(argv[i], "--height")) {
      if (argc <= i + 1) {
        printf("The %s flag requires an integer argument to be passed in.\n", argv[i]);
        return EXIT_FAILURE;
      }
      frame_height = (size_t)atoi(argv[i + 1]) - 1;
    } else if (xstreq(argv[i], "--version") || xstreq(argv[i], "-v")) {
      printf("nFace version: %s\n", PROJECT_VERSION);
      return EXIT_SUCCESS;
    } else if (xstreq(argv[i], "--increment") || xstreq(argv[i], "-i")) {
      if (argc <= i + 1) {
        printf("The %s flag requires an integer argument to be passed in.\n", argv[i]);
        return EXIT_FAILURE;
      }
      block_increment = (size_t)atoi(argv[i + 1]);
    }
    else {
      print_help();
      return EXIT_SUCCESS;
    }
  }

  init_ncurses();

  WINDOW *image_frame = create_window(image_width, image_height);
  int camera_fd = open_camera();
  if (camera_fd == -1) {
    perror("Opening camera");
    goto error;
  }

  struct v4l2_format fmt = {0};
  if (set_format(camera_fd, image_width, image_height, &fmt) == -1) {
    perror("Setting format");
    goto error;
  }

  if (request_buf(camera_fd) == -1) {
    perror("Requesting buffer");
    goto error;
  }

  struct v4l2_buffer buf;
  if (get_device_info(camera_fd, &buf) == -1) {
    perror("Fetching device info");
    goto error;
  }

  unsigned char *yuyv = map_memory(camera_fd, &buf);
  if (yuyv == MAP_FAILED) {
    perror("Mapping memory");
    goto error;
  }

  if (start_stream(camera_fd) == -1) {
    perror("Start streaming");
    munmap(yuyv, buf.length);
    goto error;
  }

  size_t max_height, max_width;
  getmaxyx(image_frame, max_height, max_width);

  if (frame_height == 0 || frame_height > max_height)
    frame_height = max_height;

  if (frame_width == 0 || frame_width > max_width)
    frame_width = max_width;

  while (getch() != 'q') {
    yuyv = map_memory(camera_fd, &buf);
    if (yuyv == MAP_FAILED) {
      perror("Mapping memory");
      goto error;
    }

    if (queue_buf(camera_fd) == -1) {
      perror("Queueing buffer");
      munmap(yuyv, buf.length);
      goto error;
    }

    if (select_frame(camera_fd) == -1) {
      perror("Selecting frame");
      munmap(yuyv, buf.length);
      goto error;
    }

    if (dequeue_buf(camera_fd, &buf) == -1) {
      perror("Dequeueing buffer");
      munmap(yuyv, buf.length);
      goto error;
    }

    BMP_image *image = calloc(1, sizeof(BMP_image));
    if (image == NULL) {
      perror("malloc");
      munmap(yuyv, buf.length);
      free(image->data);
      free(image);
      goto error;
    }

    unsigned char *info_header = get_image_header(image_width, image_height);
    memcpy(&image->header, info_header, sizeof(*info_header));

    image->width = image_width;
    image->height = image_height;

    size_t dataSize = image_width * image_height * 3;

    write_image_data(image, yuyv, dataSize, fmt.fmt.pix.bytesperline);

    if (image->data == NULL) {
      perror("malloc");
      munmap(yuyv, buf.length);
      free(image);
      free(info_header);
      goto error;
    }

    werase(image_frame);

    render_ASCII(image_frame, image, frame_width, frame_height, block_increment);

    wrefresh(image_frame);

    munmap(yuyv, buf.length);
    free(info_header);
    free(image->data);
    free(image);
  }

  if (end_stream(camera_fd) == -1) {
    perror("Ending stream");
    close(camera_fd);
    delwin(image_frame);
    endwin();
    return EXIT_FAILURE;
  }

  close(camera_fd);
  delwin(image_frame);
  endwin();
  return EXIT_SUCCESS;

error:
  if (end_stream(camera_fd) == -1)
    perror("Ending stream");

  close(camera_fd);
  delwin(image_frame);
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

WINDOW *create_window(size_t image_width, size_t image_height) {
  int termRows, termCols;
  getmaxyx(stdscr, termRows, termCols);
  size_t st_term_rows = (size_t)termRows;
  size_t st_term_cols = (size_t)termCols;

  size_t scaled_image_height = image_height / HEIGHT_WIDTH_RATIO;

  size_t frame_width = (image_width <= st_term_cols) ? image_width : st_term_cols;
  size_t frame_height =
      (scaled_image_height <= st_term_rows) ? scaled_image_height : st_term_rows;

  // Centering calculations
  size_t start_y = (st_term_rows - frame_height) / 2;
  size_t start_x = (st_term_cols - frame_width) / 2;

  return newwin(frame_height, frame_width, start_y, start_x);
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
