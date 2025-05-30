#include "camera.h"

int setFormat(const int cameraFd, const size_t imageWidth,
                     const size_t imageHeight, struct v4l2_format *p_fmt) {
  p_fmt->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  p_fmt->fmt.pix.height = imageHeight;
  p_fmt->fmt.pix.width = imageWidth;
  p_fmt->fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
  p_fmt->fmt.pix.field = V4L2_FIELD_NONE;
  return xioctl(cameraFd, VIDIOC_S_FMT, p_fmt);
}
