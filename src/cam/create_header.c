#include "camera.h"
#include <stdlib.h>
#include <string.h>

unsigned char *getImageHeader(const size_t imageWidth,
                                     const size_t imageHeight) {
  unsigned char *infoHeader =
      (unsigned char *)malloc(40 * sizeof(unsigned char));
  unsigned char temp[40] = {
      40, 0, 0, 0, // Header size
      0,  0, 0, 0, // Image width
      0,  0, 0, 0, // Image height
      1,  0,       // biPlanes
      24, 0, 0, 0, // 24 bits per pixel
      0,  0, 0, 0, // Compression method (none)
      0,  0, 0, 0, // biSizeImage
      0,  0, 0, 0, // Printing related
      0,  0, 0, 0, // Printing related
      0,  0, 0, 0, // Color importancy
  };

  memcpy(infoHeader, temp, sizeof(temp));

  infoHeader[4] = (unsigned char)(imageWidth);
  infoHeader[5] = (unsigned char)(imageWidth >> 8);
  infoHeader[6] = (unsigned char)(imageWidth >> 16);
  infoHeader[7] = (unsigned char)(imageWidth >> 24);

  infoHeader[8] = (unsigned char)(imageHeight);
  infoHeader[9] = (unsigned char)(imageHeight >> 8);
  infoHeader[10] = (unsigned char)(imageHeight >> 16);
  infoHeader[11] = (unsigned char)(imageHeight >> 24);

  size_t rowSize = (imageWidth * 3 + 3) & ~3;
  size_t imageSize = rowSize * imageHeight;
  infoHeader[20] = (unsigned char)(imageSize);
  infoHeader[21] = (unsigned char)(imageSize >> 8);
  infoHeader[22] = (unsigned char)(imageSize >> 16);
  infoHeader[23] = (unsigned char)(imageSize >> 24);
  return infoHeader;
}
