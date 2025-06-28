#include "camera.h"

int start_stream(int camera_fd) {
  unsigned int stream_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fflush(stdout);
  return xioctl(camera_fd, VIDIOC_STREAMON, &stream_type);
}
