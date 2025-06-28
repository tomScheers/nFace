#include "camera.h"

int request_buf(const int cameraFd) {
  struct v4l2_requestbuffers req = {0};
  req.count = 1;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;
  return xioctl(cameraFd, VIDIOC_REQBUFS, &req);
}
