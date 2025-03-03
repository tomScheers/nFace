#include "convert_bmp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <turbojpeg.h>

static unsigned char *jpegToRawRGB(const char *jpegPath, size_t *width,
                                   size_t *height) {
  FILE *file = fopen(jpegPath, "rb");

  if (file == NULL) {
    fprintf(stderr, "Failed to open JPEG\n");
    exit(EXIT_FAILURE);
  }

  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  unsigned char* compressedImg = (unsigned char *)malloc(file_size);
  if (compressedImg == NULL) {
    perror("malloc");
    fclose(file);
    exit(EXIT_FAILURE);
  }

  size_t bytesRead = fread(compressedImg, 1, file_size, file);
  if ((long)bytesRead != file_size) {
    perror("fread");
    free(compressedImg);
    fclose(file);
    exit(EXIT_FAILURE);
  }

  tjhandle jpegDecompressor = tjInitDecompress();
  size_t jpegWidth, jpegHeight;
  if (tjDecompressHeader(jpegDecompressor, compressedImg, file_size, (int*)&jpegWidth, (int*)&jpegHeight)) {
    perror("decompressing header");
    free(compressedImg);
    fclose(file);
    exit(EXIT_FAILURE);
  }
  size_t imgWidth = 1920, imgHeight = 1080; // Temporary hardcode
  unsigned char *rawBGR = malloc(imgWidth * imgHeight * 3);

  tjDecompress2(jpegDecompressor, compressedImg, imgWidth * imgHeight, rawBGR, imgWidth, 0, imgHeight, TJPF_BGR, TJFLAG_FASTDCT);

  *width = imgWidth;
  *height = imgHeight;
  return rawBGR;
}

void convertJPEGtoBMP(const char *jpegPath, const char *bmpPath) {
  size_t width, height;
  unsigned char *rawRGB = jpegToRawRGB(jpegPath, &width, &height);

  if (!rawRGB) {
    fprintf(stderr, "Couldn't fetch RGB data\n");
    exit(EXIT_FAILURE);
  }

  size_t fileSize = width * height * 3 + 54 + (4 - (width * 3) % 4) % 4 * height;

  unsigned char fileHeader[14] = {
      'B', 'M',       // File type
      0,   0,   0, 0, // Total file size in bytes
      0,   0,         // Reserved
      0,   0,         // Reserved
      54,  0,  0, 0, // Offset to pixel data (54 bytes)
  };
  fileHeader[2] = (unsigned char)(fileSize);
  fileHeader[3] = (unsigned char)(fileSize >> 8);
  fileHeader[4] = (unsigned char)(fileSize >> 16);
  fileHeader[5] = (unsigned char)(fileSize >> 24);

  unsigned char infoHeader[40] = {
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
  infoHeader[4] = (unsigned char)(width);
  infoHeader[5] = (unsigned char)(width >> 8);
  infoHeader[6] = (unsigned char)(width >> 16);
  infoHeader[7] = (unsigned char)(width >> 24);

  infoHeader[8] = (unsigned char)(height);
  infoHeader[9] = (unsigned char)(height >> 8);
  infoHeader[10] = (unsigned char)(height >> 16);
  infoHeader[11] = (unsigned char)(height >> 24);

  FILE *f = fopen(bmpPath, "wb");
  if (!f) {
    free(rawRGB);
    exit(EXIT_FAILURE);
  }

  fwrite(fileHeader, 1, 14, f);
  fwrite(infoHeader, 1, 40, f);

  unsigned char padding[3] = {0, 0, 0};
  size_t paddingSize = (4 - (width * 3) % 4) % 4;

  for (size_t i = 0; i < height; ++i) {
    unsigned char *row = rawRGB + (width * (height - i - 1) * 3);
    for (size_t j = 0; j < width; ++j) {
      unsigned char pixel[3] = {row[j * 3 + 2], row[j * 3 + 1], row[j * 3]}; // Swap R and B
      fwrite(pixel, 1, 3, f);
    }
    fwrite(padding, 1, paddingSize, f);
  }

  free(rawRGB);
  fclose(f);
}
