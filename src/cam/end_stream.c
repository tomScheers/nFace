#include "camera.h"

int endStream(const int cameraFd) {
  unsigned int streamType = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  return xioctl(cameraFd, VIDIOC_STREAMOFF, &streamType);
}
