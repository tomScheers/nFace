#include "ita.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

BMPImg loadBMP(const char *fileName) {
  BMPImg img;
  FILE *file = fopen(fileName, "rb");
  if (!file) {
    perror("Error opening file");
    exit(EXIT_FAILURE);
  }

  fread(img.header, 1, 54, file);

  img.width = (img.header[18]) | (img.header[19] << 8) |
              (img.header[20] << 16) | (img.header[21] << 24);
  img.height = (img.header[22]) | (img.header[23] << 8) |
               (img.header[24] << 16) | (img.header[25] << 24);

  fprintf(stdout, "Image dimensions: %d x %d\n", img.width, img.height);

  int dataSize = img.width * img.height * 3;
  img.data = (unsigned char *)malloc(dataSize);
  fseek(file, 54, SEEK_SET);
  fread(img.data, 1, dataSize, file);
  fclose(file);

  return img;
}

int getBlockBrightness(BMPImg *img, int startX, int startY, int blockWidth,
                       int blockHeight) {
  int totalBrightness = 0;
  size_t count = 0;

  for (int y = startY; y < startY + blockHeight; ++y) {
    for (int x = startX; x < startX + blockWidth; ++x) {
      int index = (y * img->width + x) * 3;
      unsigned char b = img->data[index];
      unsigned char g = img->data[index + 1];
      unsigned char r = img->data[index + 2];

      int brightness = (0.2126 * r) + (0.7152 * g) + (0.0722 * b);
      totalBrightness += brightness;
      ++count;
    }
  }
  return totalBrightness / count;
}

char brightnessToASCII(int brightness) {
  const char *asciiChars = "@%#*+=-:.,";
  int index = (brightness * strlen(asciiChars)) / 255;
  return asciiChars[9 - index];
}

void renderASCII(BMPImg *img, int ASCIIWidth, int ASCIIHeight) {
  int blockWidth = img->width / ASCIIWidth;
  int blockHeight = img->height / ASCIIHeight;
  for (int y = ASCIIHeight; y >= 0; --y) {
    for (int x = 0; x < ASCIIWidth; ++x) {
      int brightness = getBlockBrightness(img, blockWidth * x, blockHeight * y,
                                          blockWidth, blockHeight);
      fprintf(stdout, "%c", brightnessToASCII(brightness));
    }
    fprintf(stdout, "\n");
  }
}
