#include "camera.h"
#include "ita.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

static WINDOW* createWindow(size_t imageWidth, size_t imageHeight);
static int xioctl(int fh, int request, void *arg);
static int openCamera();
static int setFormat(int *cameraFd, size_t imageWidth, size_t imageHeight,
                     struct v4l2_format *p_fmt);
static int requestBuffer(int *cameraFd);
static int queueBuffer(int *cameraFd);
static int startStream(int *cameraFd);
static int getDeviceInfo(int *cameraFd, struct v4l2_buffer *infoBuf);
static unsigned char *mapMemory(int *cameraFd, struct v4l2_buffer *buf);
static int selectFrame(int *cameraFd);
static int dequeueBuf(int *cameraFd, struct v4l2_buffer *buf);
static int endStream(int *cameraFd);
static unsigned char *getImageHeader(const size_t imageWidth,
                                     const size_t imageHeight);
static int writeImageData(BMPImg *image, unsigned char *yuyvData,
                          size_t dataSize, size_t stride);

void startCamera(size_t imageWidth, size_t imageHeight) {
  WINDOW* imageFrame = createWindow(imageWidth, imageHeight);
  int ch;
  int cameraFd = openCamera();
  if (cameraFd == -1) {
    exit(EXIT_FAILURE);
  }

  struct v4l2_format fmt = {0};
  if (setFormat(&cameraFd, imageWidth, imageHeight, &fmt) == -1) {
    close(cameraFd);
    exit(EXIT_FAILURE);
  }

  if (requestBuffer(&cameraFd) == -1) {
    close(cameraFd);
    exit(EXIT_FAILURE);
  }

  struct v4l2_buffer buf;
  if (getDeviceInfo(&cameraFd, &buf) == -1) {
    close(cameraFd);
    exit(EXIT_FAILURE);
  }

  unsigned char *yuyv = mapMemory(&cameraFd, &buf);
  if (yuyv == MAP_FAILED) {
    close(cameraFd);
    exit(EXIT_FAILURE);
  }

  if (startStream(&cameraFd) == -1) {
    munmap(yuyv, buf.length);
    close(cameraFd);
    exit(EXIT_FAILURE);
  }

  size_t frameWidth, frameHeight;
  getmaxyx(imageFrame, frameHeight, frameWidth);

  while ((ch = getch()) != 'q') {
    yuyv = mapMemory(&cameraFd, &buf);
    if (yuyv == MAP_FAILED) {
      close(cameraFd);
      exit(EXIT_FAILURE);
    }

    if (queueBuffer(&cameraFd) == -1) {
      munmap(yuyv, buf.length);
      close(cameraFd);
      exit(EXIT_FAILURE);
    }

    if (selectFrame(&cameraFd) == -1) {
      munmap(yuyv, buf.length);
      close(cameraFd);
      exit(EXIT_FAILURE);
    }

    if (dequeueBuf(&cameraFd, &buf) == -1) {
      munmap(yuyv, buf.length);
      close(cameraFd);
      exit(EXIT_FAILURE);
    }

    BMPImg *image = malloc(sizeof(BMPImg));

    unsigned char *infoHeader = getImageHeader(imageWidth, imageHeight);
    memcpy(image->header, infoHeader, 54);

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

  if (endStream(&cameraFd) == -1) {
    close(cameraFd);
    exit(EXIT_FAILURE);
  }

  close(cameraFd);
  delwin(imageFrame);
  endwin();
}

static WINDOW* createWindow(size_t imageWidth, size_t imageHeight) {
  int startY, startX, rows, cols;
  getmaxyx(stdscr, rows, cols);

  if (imageWidth < cols) {
    cols = imageWidth;
  }
  if (imageHeight < rows) {
    rows = imageHeight;
  }

  int factor = (cols < rows) ? cols : rows;
  fprintf(stdout, "factor: %d\n", factor);
  float ratio = (float)imageWidth / (float)imageHeight;
  float blockWidthToHeightRatio =
      2.0; // The ratio of a chars width in the terminal to it's height
  size_t frameHeight = rows;
  size_t frameWidth =
      (size_t)((frameHeight * ratio + 0.5) * blockWidthToHeightRatio);

  startY = rows / 2 - frameHeight / 2;
  startX = cols / 2 - frameWidth / 2;

  return newwin(frameHeight, frameWidth, startY, startX);
}


static int xioctl(int fh, int request, void *arg) {
  int r;
  do {
    r = ioctl(fh, request, arg);
  } while (r == -1 && EINTR == errno);
  return r;
}

static int openCamera() { return open("/dev/video0", O_RDWR); }

static int setFormat(int *cameraFd, size_t imageWidth, size_t imageHeight,
                     struct v4l2_format *p_fmt) {
  // struct v4l2_format fmt = *p_fmt;
  p_fmt->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  p_fmt->fmt.pix.height = imageHeight;
  p_fmt->fmt.pix.width = imageWidth;
  p_fmt->fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
  p_fmt->fmt.pix.field = V4L2_FIELD_NONE;
  return xioctl(*cameraFd, VIDIOC_S_FMT, p_fmt);
}

static int requestBuffer(int *cameraFd) {
  struct v4l2_requestbuffers req = {0};
  req.count = 1;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;
  return xioctl(*cameraFd, VIDIOC_REQBUFS, &req);
}

static int queueBuffer(int *cameraFd) {
  struct v4l2_buffer buf = {0};
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;
  buf.index = 0;
  return xioctl(*cameraFd, VIDIOC_QBUF, &buf);
}

static int startStream(int *cameraFd) {
  unsigned int streamType = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fflush(stdout);
  return xioctl(*cameraFd, VIDIOC_STREAMON, &streamType);
}

static int getDeviceInfo(int *cameraFd, struct v4l2_buffer *infoBuf) {
  infoBuf->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  infoBuf->memory = V4L2_MEMORY_MMAP;
  infoBuf->index = 0;
  return xioctl(*cameraFd, VIDIOC_QUERYBUF, infoBuf);
}

static unsigned char *mapMemory(int *cameraFd, struct v4l2_buffer *buf) {
  return (unsigned char *)mmap(NULL, buf->length, PROT_READ | PROT_WRITE,
                               MAP_SHARED, *cameraFd, buf->m.offset);
}

static int selectFrame(int *cameraFd) {
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(*cameraFd, &fds);
  struct timeval tv = {0};
  tv.tv_sec = 5;
  return select((*cameraFd) + 1, &fds, NULL, NULL, &tv);
}

static int dequeueBuf(int *cameraFd, struct v4l2_buffer *buf) {
  return xioctl(*cameraFd, VIDIOC_DQBUF, buf);
}

static int endStream(int *cameraFd) {
  unsigned int streamType = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  return xioctl(*cameraFd, VIDIOC_STREAMOFF, &streamType);
}

static unsigned char *getImageHeader(const size_t imageWidth,
                                     const size_t imageHeight) {
  unsigned char *infoHeader =
      (unsigned char *)malloc(40 * sizeof(unsigned char));
  unsigned char temp[40] = {
      40, 0, 0, 0, // Header size
      0,  0, 0, 0, // Image width
      0,  0, 0, 0, // Image height
      1,  0,       // biPlanes
      24, 0, 0, 0, // 24 bits per pixel
      0,  0, 0, 0, // Compression method (none)
      0,  0, 0, 0, // biSizeImage
      0,  0, 0, 0, // Printing related
      0,  0, 0, 0, // Printing related
      0,  0, 0, 0, // Color importancy
  };

  memcpy(infoHeader, temp, sizeof(temp));

  infoHeader[4] = (unsigned char)(imageWidth);
  infoHeader[5] = (unsigned char)(imageWidth >> 8);
  infoHeader[6] = (unsigned char)(imageWidth >> 16);
  infoHeader[7] = (unsigned char)(imageWidth >> 24);

  infoHeader[8] = (unsigned char)(imageHeight);
  infoHeader[9] = (unsigned char)(imageHeight >> 8);
  infoHeader[10] = (unsigned char)(imageHeight >> 16);
  infoHeader[11] = (unsigned char)(imageHeight >> 24);

  size_t rowSize = (imageWidth * 3 + 3) & ~3;
  size_t imageSize = rowSize * imageHeight;
  infoHeader[20] = (unsigned char)(imageSize);
  infoHeader[21] = (unsigned char)(imageSize >> 8);
  infoHeader[22] = (unsigned char)(imageSize >> 16);
  infoHeader[23] = (unsigned char)(imageSize >> 24);
  return infoHeader;
}

static int writeImageData(BMPImg *image, unsigned char *yuyvData,
                          size_t dataSize, size_t stride) {
  size_t size = 0;
  image->data = malloc(dataSize);

  for (size_t i = 0; i < (size_t)image->height; ++i) {
    unsigned char *row =
        yuyvData + stride * (image->height - i - 1); // Adjust row pointer
    for (size_t j = 0; j < (size_t)image->width / 2; ++j) {
      int Y0 = row[j * 4 + 0];      // Y0
      int U = row[j * 4 + 1] - 128; // U0 (shared by two pixels)
      int Y1 = row[j * 4 + 2];      // Y1
      int V = row[j * 4 + 3] - 128; // V0 (shared by two pixels)

      // Convert first pixel (Y0, U, Y1)
      int R0 = Y0 + ((179 * V) >> 8);
      int G0 = Y0 - ((44 * U + 91 * V) >> 8);
      int B0 = Y0 + ((227 * U) >> 8);

      // Convert second pixel (Y1, U, V)
      int R1 = Y1 + ((179 * V) >> 8);
      int G1 = Y1 - ((44 * U + 91 * V) >> 8);
      int B1 = Y1 + ((227 * U) >> 8);

      // Ensure values are within range
      R0 = R0 < 0 ? 0 : (R0 > 255 ? 255 : R0);
      G0 = G0 < 0 ? 0 : (G0 > 255 ? 255 : G0);
      B0 = B0 < 0 ? 0 : (B0 > 255 ? 255 : B0);

      R1 = R1 < 0 ? 0 : (R1 > 255 ? 255 : R1);
      G1 = G1 < 0 ? 0 : (G1 > 255 ? 255 : G1);
      B1 = B1 < 0 ? 0 : (B1 > 255 ? 255 : B1);

      // Write the first pixel (BGR)
      unsigned char pixel0[3] = {B0, G0, R0};
      image->data[size++] = pixel0[0];
      image->data[size++] = pixel0[1];
      image->data[size++] = pixel0[2];

      // Write the second pixel (BGR)
      unsigned char pixel1[3] = {B1, G1, R1};
      image->data[size++] = pixel1[0];
      image->data[size++] = pixel1[1];
      image->data[size++] = pixel1[2];
    }
  }
  return 1;
}
