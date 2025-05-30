#include "camera.h"

int selectFrame(const int cameraFd) {
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(cameraFd, &fds);
  struct timeval tv = {0};
  tv.tv_sec = 5;
  return select((cameraFd) + 1, &fds, NULL, NULL, &tv);
}
