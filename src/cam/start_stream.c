#include "camera.h"

int startStream(const int cameraFd) {
  unsigned int streamType = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fflush(stdout);
  return xioctl(cameraFd, VIDIOC_STREAMON, &streamType);
}
