#include "rgbimage.hpp"

#include <jpeglib.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>

#include <chrono>
#include <cstring>
#include <iostream>
#include <string>

void rgbimage::putpixel(int x, int y, uint8_t *color) {
  if (((x < w) && (x >= 0)) && ((y < h) && (y >= 0))) {
    memcpy(buffer + 3 * (x + y * w), color, 3);
  }
}

uint8_t *rgbimage::getpixel(int x, int y) const {
  if (((x < w) && (x >= 0)) && ((y < h) && (y >= 0))) {
    return &buffer[x * 3 + y * w * 3];
  } else {
    return NULL;
  }
}

uint8_t *rgbimage::getpixel_inf(int x, int y) const {
  if (((x < w) && (x >= 0)) && ((y < h) && (y >= 0))) {
    return &buffer[x * 3 + y * w * 3];
  } else {
    return &buffer[0];
  }
}

void rgbimage::addalpha(uint8_t value) {
  if (alpha == NULL) {
    alpha = (uint8_t *)malloc(w * h);
    memset(alpha, value, w * h);
  }
}

void rgbimage::visible_to_alpha() {
  if (alpha != NULL) {
    for (int j = 0; j < (w * h); j++) {
      alpha[j] = (buffer[3 * j] + buffer[3 * j + 1] + buffer[3 * j + 2]) / 3;
    }
  }
}

void rgbimage::alpha_to_visible() {
  if (alpha != NULL) {
    for (int j = 0; j < (w * h); j++) {
      buffer[3 * j] = alpha[j];
      buffer[3 * j + 1] = alpha[j];
      buffer[3 * j + 2] = alpha[j];
    }
  }
}

void rgbimage::setcolor(uint8_t r, uint8_t g, uint8_t b) {
  for (int j = 0; j < w * h; j++) {
    buffer[j * 3] = r;
    buffer[j * 3 + 1] = g;
    buffer[j * 3 + 2] = b;
  }
}

rgbimage::rgbimage(int w, int h) {
  buffer = NULL;
  alpha = NULL;

  this->w = w;
  this->h = h;

  buffer = (uint8_t *)malloc(3 * w * h);
  jpeg_compression = 95;
  memset(buffer, 0x00, 3 * w * h);
}

rgbimage::rgbimage() {
  buffer = NULL;
  alpha = NULL;
  jpeg_compression = 95;
  w = h = -1;
}

void rgbimage::frombuffer(uint8_t *buffer, int w, int h) {
  if ((this->w == w) and (this->h == h)) {
    memcpy(this->buffer, buffer, 3 * w * h);
  } else {
    this->w = w;
    this->h = h;
    free(this->buffer);
    this->buffer = (uint8_t *)malloc(3 * w * h);
    memcpy(this->buffer, buffer, 3 * w * h);
  }
}

void rgbimage::save_jpeg(std::string filename) {
  FILE *outfile;
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);
  if ((outfile = fopen(filename.c_str(), "wb")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename.c_str());
  }
  jpeg_stdio_dest(&cinfo, outfile);

  cinfo.image_width = w & -1;
  cinfo.image_height = h & -1;
  cinfo.input_components = 3;
  cinfo.in_color_space = JCS_RGB;

  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, this->jpeg_compression, TRUE);
  jpeg_start_compress(&cinfo, TRUE);

  uint8_t *tmprowbuf = (uint8_t *)malloc(w * 3);

  JSAMPROW row_pointer[1];
  row_pointer[0] = &tmprowbuf[0];
  uint64_t offset;
  while (cinfo.next_scanline < cinfo.image_height) {
    offset = cinfo.next_scanline * cinfo.image_width * 3;
    memcpy(tmprowbuf, this->buffer + offset, this->w * 3);
    jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);

  free(tmprowbuf);
  tmprowbuf = NULL;
  fclose(outfile);
}

void rgbimage::save_ppm(std::string filename) {
  FILE *outfile;
  char *header = (char *)malloc(1024);

  if ((outfile = fopen(filename.c_str(), "wb")) == NULL) {
    std::cout << "Can't open " << filename.c_str() << std::endl;
    exit(1);
  }

  sprintf(header, "P6 %d %d 255 ", this->w, this->h);
  fwrite(header, strlen(header), 1, outfile);
  fwrite(this->buffer, 3 * this->w * this->h, 1, outfile);
  fclose(outfile);
  free(header);
  header = NULL;
}

void rgbimage::write_jpeg_mem(unsigned char **outbuffer,
                              unsigned long *outbytes) {
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);
  jpeg_mem_dest(&cinfo, outbuffer, outbytes);

  cinfo.image_width = this->w;
  cinfo.image_height = this->h;
  cinfo.input_components = 3;
  cinfo.in_color_space = JCS_RGB;

  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, this->jpeg_compression, TRUE);
  jpeg_start_compress(&cinfo, TRUE);

  JSAMPROW row_pointer[1];
  uint64_t offset = 0;
  while (cinfo.next_scanline < cinfo.image_height) {
    offset = cinfo.next_scanline * cinfo.image_width * 3;
    row_pointer[0] = (this->buffer + offset);
    jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);
}

rgbimage::~rgbimage() {
  free(buffer);
  buffer = NULL;
  free(alpha);
  alpha = NULL;
}

void rgbimage::paste(rgbimage *img, int x, int y) {
  int src_xoffset = 0;
  int src_yoffset = 0;
  int src_w = img->get_w();
  int src_h = img->get_h();

  if ((src_w == 0) && (src_h == 0)) {
    return;
  }

  if (x < 0) {
    src_xoffset = -x;
    src_w = src_w + x;
    x = 0;
  }
  if (y < 0) {
    src_yoffset = -y;
    src_h = src_h + y;
    y = 0;
  }

  if ((x + src_w) > (w)) {
    src_w = src_w - (x + src_w - w);
  }
  if ((y + src_h) > (h)) {
    src_h = src_h - (y + src_h - h);
  }

  if (img->getAlpha() == NULL) {
    for (int r = 0; r < src_h; r++) {
      memcpy(buffer + 3 * ((r + y) * w + x),
             img->getBuffer() +
                 3 * (((r + src_yoffset) * img->get_w()) + src_xoffset),
             src_w * 3);
    }
  } else {
    uint8_t a;
    uint8_t *srccolor;
    uint8_t *dstcolor;
    uint8_t outcolor[3];

    unsigned short r, g, b;

    int dstx, dsty;
    for (int srcy = 0; srcy < src_h; srcy++) {
      for (int srcx = 0; srcx < src_w; srcx++) {
        dstx = srcx + x;
        dsty = srcy + y;
        a = img->getAlpha()[srcx + img->get_w() * srcy];
        dstcolor = getpixel(dstx, dsty);
        srccolor = img->getpixel(srcx, srcy);
        if (a == 0) {
          outcolor[0] = dstcolor[0];
          outcolor[1] = dstcolor[1];
          outcolor[2] = dstcolor[2];
        } else if (a == 255) {
          outcolor[0] = srccolor[0];
          outcolor[1] = srccolor[1];
          outcolor[2] = srccolor[2];
        } else {
          r = (uint8_t)((a / 255.0) * srccolor[0] +
                        (1.0 - (a / 255.0)) * dstcolor[0]);
          g = (uint8_t)((a / 255.0) * srccolor[1] +
                        (1.0 - (a / 255.0)) * dstcolor[1]);
          b = (uint8_t)((a / 255.0) * srccolor[2] +
                        (1.0 - (a / 255.0)) * dstcolor[2]);
          if (r > 255) {
            outcolor[0] = 0xFF;
          } else {
            outcolor[0] = r;
          }
          if (g > 255) {
            outcolor[1] = 0xFF;
          } else {
            outcolor[1] = g;
          }
          if (b > 255) {
            outcolor[2] = 0xFF;
          } else {
            outcolor[2] = b;
          }
        }
        putpixel(dstx + src_xoffset, dsty + src_yoffset, outcolor);
      }
    }
  }
}

void rgbimage::expand(int radius) {
  uint8_t *color = NULL;
  rgbimage temp = rgbimage(w + 2 * radius, h + 2 * radius);

  uint8_t white[3] = {0xFF, 0xFF, 0xFF};

  for (int ky = 0; ky < h; ky++) {
    for (int kx = 0; kx < w; kx++) {
      uint8_t *tempcolor = NULL;

      color = getpixel(kx, ky);
      tempcolor = temp.getpixel(kx, ky);

      for (int cy = 0; cy < 2 * radius; cy++) {
        for (int cx = 0; cx < 2 * radius; cx++) {
          if (tempcolor != NULL) {
            if (color[0] > 128) {
              int rx = (cx - radius);
              int ry = (cy - radius);
              if (rx * rx + ry * ry < (radius * radius)) {
                temp.putpixel(cx + kx, cy + ky, white);
              }
            }
          }
        }
      }
    }
  }

  w = temp.get_w();
  h = temp.get_h();
  free(buffer);
  buffer = (uint8_t *)malloc(3 * w * h);
  memcpy(buffer, temp.getBuffer(), 3 * w * h);
}

void rgbimage::gaussianblur(double stddev) {
  rgbimage kernel = mkgaussiankernel(stddev, 0.0, 25);
  convolve(&kernel);
}

rgbimage rgbimage::mkgaussiankernel(double stddev, double avg, int radius) {
  int size = 2 * radius + 1;

  rgbimage kernel = rgbimage(size, size);
  kernel.addalpha(0);

  uint8_t color[3] = {0, 0, 0};

  double max =
      exp(-0.5 * pow(-avg / stddev, 2.0)) / (sqrt(2.0 * M_PI) * stddev);

  for (int y = 0; y < size; y++) {
    for (int x = 0; x < size; x++) {
      double halfsize = (size) / 2;
      double dist = sqrt(pow(x - halfsize, 2.0) + pow(y - halfsize, 2.0));
      double exponent = -0.5 * pow((dist - avg) / stddev, 2.0);

      double gval = exp(exponent) / (sqrt(2.0 * M_PI) * stddev);
      uint8_t outval = 0;
      if (gval > 255.0) {
        outval = 0xFF;
      } else {
        outval = (uint8_t)(gval * (255.0 / max));
      }
      color[0] = color[1] = color[2] = outval;
      kernel.putpixel(x, y, color);
    }
  }

  // Minimize the kernel size
  kernel.autocrop();

  return kernel;
}

void rgbimage::convolve(rgbimage *kernel) {
  int kw = kernel->w;
  int kh = kernel->h;

  rgbimage temp = rgbimage(w + kw, h + kh);

  unsigned long kernelsum = 0;

  for (int ky = 0; ky < kh; ky++) {
    for (int kx = 0; kx < kw; kx++) {
      kernelsum += kernel->getpixel_inf(kx, ky)[0];
    }
  }

  for (int iy = -(kh / 2); iy < h + (kh / 2); iy++) {
    for (int ix = -(kw / 2); ix < w + (kw / 2); ix++) {
      unsigned long rsum = 0;
      unsigned long gsum = 0;
      unsigned long bsum = 0;
      uint8_t *kernelcolor;
      uint8_t *imgcolor;
      uint8_t outcolor[3];

      for (int ky = 0; ky < kh + (kw / 2); ky++) {
        for (int kx = -(kh / 2); kx < kw + (kw / 2); kx++) {
          imgcolor = getpixel_inf(ix - kx + (kw / 2), iy - ky + (kh / 2));
          kernelcolor = kernel->getpixel_inf(kx, ky);

          rsum += imgcolor[0] * kernelcolor[0];
          gsum += imgcolor[1] * kernelcolor[1];
          bsum += imgcolor[2] * kernelcolor[2];
        }
      }

      outcolor[0] = rsum / (kernelsum);
      outcolor[1] = gsum / (kernelsum);
      outcolor[2] = bsum / (kernelsum);

      temp.putpixel(ix + (kw / 2), iy + (kh / 2), outcolor);
    }
  }

  w = w + kw;
  h = h + kh;

  free(buffer);

  buffer = (uint8_t *)malloc(3 * w * h);
  memcpy(buffer, temp.getBuffer(), 3 * w * h);
}

void rgbimage::crop(int x0, int y0, int x1, int y1) {
  int new_w = x1 - x0;
  int new_h = y1 - y0;

  uint8_t *srcrow = buffer + (3 * (w * y0 + x0));
  uint8_t *newbuffer = (uint8_t *)malloc(3 * new_w * new_h);
  uint8_t *dstrow = newbuffer;

  uint8_t *srcrow_alpha = NULL;
  uint8_t *newbuffer_alpha = NULL;
  uint8_t *dstrow_alpha = newbuffer;

  if (alpha != NULL) {
    newbuffer_alpha = (uint8_t *)malloc(new_w * new_h);
    srcrow_alpha = alpha + (w * y0 + x0);
    dstrow_alpha = newbuffer_alpha;
  }

  for (int i = 0; i < new_h; i++) {
    memcpy(dstrow, srcrow, 3 * new_w);
    srcrow += 3 * w;
    dstrow += 3 * new_w;
    if (alpha != NULL) {
      memcpy(dstrow_alpha, srcrow_alpha, new_w);
      srcrow_alpha += w;
      dstrow_alpha += new_w;
    }
  }

  w = new_w;
  h = new_h;

  free(buffer);
  buffer = newbuffer;
  free(alpha);
  alpha = newbuffer_alpha;
}

// Crops the image to the minimal not black area
void rgbimage::autocrop() {
  int minx = w;
  int miny = h;
  int maxx = 0;
  int maxy = 0;

  for (int i = 0; i < h; i++) {
    for (int j = minx; j >= 0; j--) {
      int offset = (w * i + j);

      if (buffer[offset * 3] != 0) minx = j;
      if (buffer[offset * 3 + 1] != 0) minx = j;
      if (buffer[offset * 3 + 2] != 0) minx = j;
      if (alpha != NULL) {
        if (alpha[offset] != 0) minx = j;
      }
    }
    for (int j = maxx; j < w; j++) {
      int offset = (w * i + j);

      if (buffer[offset * 3] != 0) maxx = j;
      if (buffer[offset * 3 + 1] != 0) maxx = j;
      if (buffer[offset * 3 + 2] != 0) maxx = j;
      if (alpha != NULL) {
        if (alpha[offset] != 0) maxx = j;
      }
    }
  }

  for (int i = 0; i < w; i++) {
    for (int j = miny; j >= 0; j--) {
      int offset = (i + j * w);

      if (buffer[offset * 3] != 0) miny = j;
      if (buffer[offset * 3 + 1] != 0) miny = j;
      if (buffer[offset * 3 + 2] != 0) miny = j;
      if (alpha != NULL) {
        if (alpha[offset] != 0) miny = j;
      }
    }
    for (int j = maxy; j < h; j++) {
      int offset = (i + j * w);

      if (buffer[offset * 3] != 0) maxy = j;
      if (buffer[offset * 3 + 1] != 0) maxy = j;
      if (buffer[offset * 3 + 2] != 0) maxy = j;
      if (alpha != NULL) {
        if (alpha[offset] != 0) maxy = j;
      }
    }
  }

  maxx += 1;
  maxy += 1;

  crop(minx, miny, maxx, maxy);
}
