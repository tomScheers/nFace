#include "camera.h"

int end_stream(int cameraFd) {
  unsigned int stream_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  return xioctl(cameraFd, VIDIOC_STREAMOFF, &stream_type);
}
