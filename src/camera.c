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

void captureFrame() {
  int fd = open("/dev/video0", O_RDWR); // Opening camera

  // Setting format
  struct v4l2_format fmt = {0};
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  // Image dimensions
  fmt.fmt.pix.width = 320;
  fmt.fmt.pix.height = 240;

  fmt.fmt.pix.pixelformat =
      V4L2_PIX_FMT_JPEG; // Image format, for this program it uses jpeg
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



  // Start streaming
  unsigned int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fflush(stdout);
  if (xioctl(fd, VIDIOC_STREAMON, &type) == -1) {
    perror("VIDIOC_STREAMON");
    munmap(buffer, buf.length);
    close(fd);
    exit(1);
  }

  // Queue buffer
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

  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(fd, &fds);
  struct timeval tv = {0};
  tv.tv_sec = 2;

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

  FILE *file = fopen("output.jpeg", "wb");
  fwrite(buffer, 1, buf.bytesused, file);
  munmap(buffer, buf.length);
  fclose(file);
  close(fd);
}
