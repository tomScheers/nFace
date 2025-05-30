#include "camera.h"

int dequeueBuf(const int cameraFd, const struct v4l2_buffer *buf) {
  return xioctl(cameraFd, VIDIOC_DQBUF, (void *)buf);
}
