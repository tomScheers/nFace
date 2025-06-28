#include "camera.h"

int get_device_info(int cameraFd, struct v4l2_buffer *infoBuf) {
  infoBuf->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  infoBuf->memory = V4L2_MEMORY_MMAP;
  infoBuf->index = 0;
  return xioctl(cameraFd, VIDIOC_QUERYBUF, infoBuf);
}
