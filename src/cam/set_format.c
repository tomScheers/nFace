#include "camera.h"

int set_format(int camera_fd, size_t image_width,
                     size_t image_height, struct v4l2_format *p_fmt) {
  p_fmt->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  p_fmt->fmt.pix.height = image_height;
  p_fmt->fmt.pix.width = image_width;
  p_fmt->fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
  p_fmt->fmt.pix.field = V4L2_FIELD_NONE;
  return xioctl(camera_fd, VIDIOC_S_FMT, p_fmt);
}
