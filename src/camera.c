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
#include <ncurses.h>

/*
 * @brief Creates a ncurses window
 *
 * @details The image dimensions are derived from the paramaters
 *
 * @param imageWidth Width of the captured image (determines window width)
 * @param imageHeight Height of the captured image (determines window height)
 * @return Pointer to the created ncurses window
 */
static WINDOW *createWindow(const size_t imageWidth, const size_t imageHeight);

static int xioctl(int fh, int request, void *arg);

/*
 * @brief Opens the camera
 *
 * @details Opens the camera device at /dev/video0
 *
 * @return File decriptor for the camera device, or -1 on failure
 */
static int openCamera();

/*
 * @brief Sets the format for the camera
 *
 * @details Sets the camera format to use YUYV, and what the image dimensions
 * should be
 *
 * @param cameraFd File descriptor of the camera
 * @param imageWidth The width of the image being captured
 * @param imageHeight The height of the image being captured
 * @param *p_fmt Format to set, this is used later in the code
 * @return -1 on failure
 */
static int setFormat(const int cameraFd, const size_t imageWidth,
                     const size_t imageHeight, struct v4l2_format *p_fmt);

/*
 * @brief Request a camera buffer
 *
 * @details Request a buffer with data the camera captured
 *
 * @param cameraFd File descriptor for the camera
 * @return -1 on failure
 */
static int requestBuffer(const int cameraFd);

/*
 * @brief Queues buffer
 *
 * @details Queues buffer to camera file descriptor
 *
 * @param cameraFd File descriptor for camera
 * @return -1 on failure
 */
static int queueBuffer(const int cameraFd);

/*
 * @brief Starts video stream
 *
 * @details Start streaming video to the queued buffer
 *
 * @param cameraFd File descriptor for the camera
 * @return -1 on failure
 *
 */
static int startStream(const int cameraFd);

/* 
 * @brief Gets device information
 *
 * @details Writes information about the camera to infoBuf
 *
 * @param cameraFd File descriptor for the camera to get the information of
 * @param infoBuff Buffer to write the device information to
 * @return -1 on failure
*/
static int getDeviceInfo(const int cameraFd, struct v4l2_buffer *infoBuf);

/* 
 * @brief Maps memory to buffer
 *
 * @details Maps memory to buf using mmap
 *
 * @important Make sure to call unmap on the mapped memory!
 *
 * @param cameraFd File descriptor for the camera device
 * @param buf Buffer to map the memory to
*/
static unsigned char *mapMemory(const int cameraFd,
                                const struct v4l2_buffer *buf);

/*
* @brief Selects frame to be captured
*
* @param cameraFd File descriptor for the camera device
* @return -1 on failure
*/
static int selectFrame(const int cameraFd);
static int dequeueBuf(const int cameraFd, const struct v4l2_buffer *buf);
static int endStream(const int cameraFd);
static unsigned char *getImageHeader(const size_t imageWidth,
                                     const size_t imageHeight);
static int writeImageData(BMPImage *image, const unsigned char *yuyvData,
                          const size_t dataSize, const size_t stride);

void startCamera(const size_t imageWidth, const size_t imageHeight) {
  WINDOW *imageFrame = createWindow(imageWidth, imageHeight);
  int cameraFd = openCamera();
  if (cameraFd == -1) {
    perror("Opening camera");
    exit(EXIT_FAILURE);
  }

  struct v4l2_format fmt = {0};
  if (setFormat(cameraFd, imageWidth, imageHeight, &fmt) == -1) {
    perror("Setting format");
    close(cameraFd);
    exit(EXIT_FAILURE);
  }

  if (requestBuffer(cameraFd) == -1) {
    perror("Requesting buffer");
    close(cameraFd);
    exit(EXIT_FAILURE);
  }

  struct v4l2_buffer buf;
  if (getDeviceInfo(cameraFd, &buf) == -1) {
    perror("Fetching device info");
    close(cameraFd);
    exit(EXIT_FAILURE);
  }

  unsigned char *yuyv = mapMemory(cameraFd, &buf);
  if (yuyv == MAP_FAILED) {
    perror("Mapping memory");
    close(cameraFd);
    exit(EXIT_FAILURE);
  }

  if (startStream(cameraFd) == -1) {
    perror("Start streaming");
    munmap(yuyv, buf.length);
    close(cameraFd);
    exit(EXIT_FAILURE);
  }

  size_t frameWidth, frameHeight;
  getmaxyx(imageFrame, frameHeight, frameWidth);

  while (getch() != 'q') {
    yuyv = mapMemory(cameraFd, &buf);
    if (yuyv == MAP_FAILED) {
      perror("Mapping memory");
      close(cameraFd);
      exit(EXIT_FAILURE);
    }

    if (queueBuffer(cameraFd) == -1) {
      perror("Queueing buffer");
      munmap(yuyv, buf.length);
      close(cameraFd);
      exit(EXIT_FAILURE);
    }

    if (selectFrame(cameraFd) == -1) {
      perror("Selecting frame");
      munmap(yuyv, buf.length);
      close(cameraFd);
      exit(EXIT_FAILURE);
    }

    if (dequeueBuf(cameraFd, &buf) == -1) {
      perror("Dequeueing buffer");
      munmap(yuyv, buf.length);
      close(cameraFd);
      exit(EXIT_FAILURE);
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
    exit(EXIT_FAILURE);
  }

  close(cameraFd);
  delwin(imageFrame);
  endwin();
}

static WINDOW *createWindow(const size_t imageWidth, const size_t imageHeight) {
  int startY, startX, rows, cols;
  getmaxyx(stdscr, rows, cols);

  if ((int)imageWidth < cols) {
    cols = imageWidth;
  }
  if ((int)imageHeight < rows) {
    rows = imageHeight;
  }

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

static int setFormat(const int cameraFd, const size_t imageWidth,
                     const size_t imageHeight, struct v4l2_format *p_fmt) {
  p_fmt->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  p_fmt->fmt.pix.height = imageHeight;
  p_fmt->fmt.pix.width = imageWidth;
  p_fmt->fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
  p_fmt->fmt.pix.field = V4L2_FIELD_NONE;
  return xioctl(cameraFd, VIDIOC_S_FMT, p_fmt);
}

static int requestBuffer(const int cameraFd) {
  struct v4l2_requestbuffers req = {0};
  req.count = 1;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;
  return xioctl(cameraFd, VIDIOC_REQBUFS, &req);
}

static int queueBuffer(const int cameraFd) {
  struct v4l2_buffer buf = {0};
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;
  buf.index = 0;
  return xioctl(cameraFd, VIDIOC_QBUF, &buf);
}

static int startStream(const int cameraFd) {
  unsigned int streamType = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fflush(stdout);
  return xioctl(cameraFd, VIDIOC_STREAMON, &streamType);
}

static int getDeviceInfo(const int cameraFd, struct v4l2_buffer *infoBuf) {
  infoBuf->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  infoBuf->memory = V4L2_MEMORY_MMAP;
  infoBuf->index = 0;
  return xioctl(cameraFd, VIDIOC_QUERYBUF, infoBuf);
}

static unsigned char *mapMemory(const int cameraFd,
                                const struct v4l2_buffer *buf) {
  return (unsigned char *)mmap(NULL, buf->length, PROT_READ | PROT_WRITE,
                               MAP_SHARED, cameraFd, buf->m.offset);
}

static int selectFrame(const int cameraFd) {
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(cameraFd, &fds);
  struct timeval tv = {0};
  tv.tv_sec = 5;
  return select((cameraFd) + 1, &fds, NULL, NULL, &tv);
}

static int dequeueBuf(const int cameraFd, const struct v4l2_buffer *buf) {
  return xioctl(cameraFd, VIDIOC_DQBUF, (void *)buf);
}

static int endStream(const int cameraFd) {
  unsigned int streamType = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  return xioctl(cameraFd, VIDIOC_STREAMOFF, &streamType);
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

static int writeImageData(BMPImage *image, const unsigned char *yuyvData,
                          const size_t dataSize, const size_t stride) {
  size_t size = 0;
  image->data = malloc(dataSize);

  for (size_t i = 0; i < (size_t)image->height; ++i) {
    const unsigned char *row =
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
