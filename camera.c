#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

static int xioctl(int fd, int request, void *arg) {
  int r;
  do {
    r = ioctl(fd, request, arg);
    if (r == -1) {
      fprintf(stderr, "ioctl failed: request=0x%lx, errno=%d\n",
              (unsigned long)request, errno);
      perror("ioctl error");
    }
  } while (-1 == r && EINTR == errno);
  return r;
}

BMPImg *captureFrame() {
  int fd = open("/dev/video0", O_RDWR);
  if (fd == -1) {
    perror("Opening video device");
    exit(EXIT_FAILURE);
  }

  struct v4l2_capability caps = {0};
  if (xioctl(fd, VIDIOC_QUERYCAP, &caps) == -1) {
    perror("Querying capabilities");
    close(fd);
    exit(EXIT_FAILURE);
  }

  struct v4l2_format fmt = {0};
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width = 320;
  fmt.fmt.pix.height = 240;
  fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
  fmt.fmt.pix.field = V4L2_FIELD_NONE;

  if (xioctl(fd, VIDIOC_S_FMT, &fmt) == -1) {
    perror("Setting Pixel Format");
    close(fd);
    exit(EXIT_FAILURE);
  }

  struct v4l2_requestbuffers req = {0};
  req.count = 1;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;

  if (xioctl(fd, VIDIOC_REQBUFS, &req) == -1) {
    perror("Requesting Buffer");
    close(fd);
    exit(EXIT_FAILURE);
  }

  struct v4l2_buffer buf = {0};
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;
  buf.index = 0;

  if (xioctl(fd, VIDIOC_QUERYBUF, &buf) == -1) {
    perror("Querying Buffer");
    close(fd);
    exit(EXIT_FAILURE);
  }

  void *buffer = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
                      buf.m.offset);
  if (buffer == MAP_FAILED) {
    perror("Memory mapping buffer");
    close(fd);
    exit(EXIT_FAILURE);
  }

  // Queue the buffer before dequeuing
  if (xioctl(fd, VIDIOC_QBUF, &buf) == -1) {
    perror("Queueing Buffer");
    munmap(buffer, buf.length);
    close(fd);
    exit(EXIT_FAILURE);
  }

  if (xioctl(fd, VIDIOC_STREAMON, &buf.type) == -1) {
    perror("Start Capture");
    munmap(buffer, buf.length);
    close(fd);
    exit(EXIT_FAILURE);
  }

  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(fd, &fds);
  struct timeval tv = {0};
  tv.tv_sec = 5; // Increase timeout

  printf("Waiting for frame...\n");
  fflush(stdout);
  int r = select(fd + 1, &fds, NULL, NULL, &tv);
  if (r == -1) {
    perror("Waiting for Frame");
    munmap(buffer, buf.length);
    close(fd);
    exit(EXIT_FAILURE);
  }

  printf("Dequeuing buffer...\n");
  if (xioctl(fd, VIDIOC_DQBUF, &buf) == -1) {
    perror("Retrieving Frame");
    munmap(buffer, buf.length);
    close(fd);
    exit(EXIT_FAILURE);
  }

  fprintf(stdout, "Saving file...\n");
  FILE *file = fopen("frame.jpg", "wb");
  if (!file) {
    perror("Saving image");
    munmap(buffer, buf.length);
    close(fd);
    exit(EXIT_FAILURE);
  }

  fwrite(buffer, 1, buf.bytesused, file);
  fclose(file);

  convertJPEGtoBMP("frame.jpg", "frame.bmp");


  printf("Loading into BMp...\n");
  BMPImg *img = loadBMP("frame.bmp");

  // Clean up
  munmap(buffer, buf.length);
  close(fd);

  printf("returning image\n");
  return img;
}
