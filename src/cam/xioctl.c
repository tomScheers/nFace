#include "camera.h"

#include <sys/ioctl.h>
#include <errno.h>

int xioctl(int fh, int request, void *arg) {
  int r;
  do {
    r = ioctl(fh, request, arg);
  } while (r == -1 && EINTR == errno);
  return r;
}
