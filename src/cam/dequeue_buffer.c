#include "camera.h"

int dequeue_buf(int cameraFd, struct v4l2_buffer *buf) {
  return xioctl(cameraFd, VIDIOC_DQBUF, (void *)buf);
}
