#include "camera.h"

int queue_buf(int cameraFd) {
  struct v4l2_buffer buf = {0};
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;
  buf.index = 0;
  return xioctl(cameraFd, VIDIOC_QBUF, &buf);
}
