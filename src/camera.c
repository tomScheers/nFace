#include "camera.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

static int xioctl(int fh, int request, void *arg) {
  int r;
  do {
    r = ioctl(fh, request, arg);
  } while (r == -1 && EINTR == errno);
  return r;
}

void captureFrame(size_t img_width, size_t img_height) {
  int fd = open("/dev/video0", O_RDWR); // Opening camera

  // Setting format
  struct v4l2_format fmt = {0};
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  // Image dimensions
  fmt.fmt.pix.width = img_width;
  fmt.fmt.pix.height = img_height;

  fmt.fmt.pix.pixelformat =
      V4L2_PIX_FMT_YUYV; // Image format, for this program it uses jpeg
  fmt.fmt.pix.field = V4L2_FIELD_NONE;
  if (xioctl(fd, VIDIOC_S_FMT, &fmt) == -1) {
    perror("setting fmt");
    close(fd);
    exit(1);
  }

  // Requesting buffer
  struct v4l2_requestbuffers req = {0};
  req.count = 1; // Request 1 frame
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;
  if (xioctl(fd, VIDIOC_REQBUFS, &req) == -1) {
    perror("requesting buffer");
    close(fd);
    exit(1);
  }

  // Get information about the device
  struct v4l2_buffer buf = {0};
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;
  buf.index = 0;

  if (xioctl(fd, VIDIOC_QUERYBUF, &buf) == -1) {
    perror("querying buffer");
    close(fd);
    exit(1);
  }

  void *buffer = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
                      buf.m.offset);
  if (buffer == MAP_FAILED) {
    perror("mapping memory");
    close(fd);
    exit(1);
  }

  // Queue buffer first
  struct v4l2_buffer bufd = {0};
  bufd.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  bufd.memory = V4L2_MEMORY_MMAP;
  bufd.index = 0;
  if (xioctl(fd, VIDIOC_QBUF, &bufd) == -1) {
    perror("Queueing Buffer");
    munmap(buffer, buf.length);
    close(fd);
    exit(1);
  }

  // Start streaming
  unsigned int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fflush(stdout);
  if (xioctl(fd, VIDIOC_STREAMON, &type) == -1) {
    perror("VIDIOC_STREAMON");
    munmap(buffer, buf.length);
    close(fd);
    exit(1);
  }

  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(fd, &fds);
  struct timeval tv = {0};
  tv.tv_sec = 5;

  if (select(fd + 1, &fds, NULL, NULL, &tv) == -1) {
    perror("Selecting Frame");
    munmap(buffer, buf.length);
    close(fd);
    exit(1);
  }

  if (xioctl(fd, VIDIOC_DQBUF, &buf) == -1) {
    perror("Dequeueing buff");
    munmap(buffer, buf.length);
    close(fd);
    exit(1);
  }

  if (buf.bytesused < img_width * img_height * 2) {
    fprintf(stderr, "Warning: Incomplete frame, received %d bytes\n", buf.bytesused);
  }

  for (int i = 0; i < 16; i++) {
    fprintf(stdout, "%02X ", ((unsigned char*)buffer)[i]);
  }
  fprintf(stdout, "%c", '\n');

  unsigned char *yuyv = buffer;

  size_t fileSize = img_width * img_height * 3 + 54;

  unsigned char fileHeader[14] = {
      'B', 'M',       // File type
      0,   0,   0, 0, // Total file size in bytes
      0,   0,         // Reserved
      0,   0,         // Reserved
      54,  0,   0, 0, // Offset to pixel data (54 bytes)
  };
  fileHeader[2] = (unsigned char)(fileSize);
  fileHeader[3] = (unsigned char)(fileSize >> 8);
  fileHeader[4] = (unsigned char)(fileSize >> 16);
  fileHeader[5] = (unsigned char)(fileSize >> 24);

  unsigned char infoHeader[40] = {
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

  size_t header_width = img_width;
  infoHeader[4] = (unsigned char)(header_width);
  infoHeader[5] = (unsigned char)(header_width >> 8);
  infoHeader[6] = (unsigned char)(header_width >> 16);
  infoHeader[7] = (unsigned char)(header_width >> 24);

  size_t header_height = buf.bytesused / img_width / 2;
  infoHeader[8] = (unsigned char)(header_height);
  infoHeader[9] = (unsigned char)(header_height >> 8);
  infoHeader[10] = (unsigned char)(header_height >> 16);
  infoHeader[11] = (unsigned char)(header_height >> 24);

  size_t rowSize = (img_width * 3 + 3) & ~3;
  size_t imageSize = rowSize * header_height;
  infoHeader[20] = (unsigned char)(imageSize);
  infoHeader[21] = (unsigned char)(imageSize >> 8);
  infoHeader[22] = (unsigned char)(imageSize >> 16);
  infoHeader[23] = (unsigned char)(imageSize >> 24);

  FILE *f = fopen("output/output.bmp", "wb");
  if (!f) {
    free(yuyv);
    exit(EXIT_FAILURE);
  }

  fwrite(fileHeader, 1, 14, f);
  fwrite(infoHeader, 1, 40, f);

  unsigned char padding[3] = {0, 0, 0};
  size_t paddingSize = (4 - (img_width * 3) % 4) % 4;

  for (size_t i = 0; i < header_height; ++i) {
    size_t stride = fmt.fmt.pix.bytesperline;
    unsigned char *row =
        yuyv + stride * (header_height - i - 1); // Adjust row pointer
    for (size_t j = 0; j < header_width / 2; ++j) {
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
      fwrite(pixel0, 1, 3, f);

      // Write the second pixel (BGR)
      unsigned char pixel1[3] = {B1, G1, R1};
      fwrite(pixel1, 1, 3, f);
    }
    fwrite(padding, 1, paddingSize, f);
  }

  fclose(f);
  munmap(buffer, buf.length);
  close(fd);
}
