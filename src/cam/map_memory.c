#include "camera.h"

#include <fcntl.h>
#include <sys/mman.h>

unsigned char *map_memory(int cameraFd, const struct v4l2_buffer *buf) {
  return (unsigned char *)mmap(NULL, buf->length, PROT_READ | PROT_WRITE,
                               MAP_SHARED, cameraFd, buf->m.offset);
}
