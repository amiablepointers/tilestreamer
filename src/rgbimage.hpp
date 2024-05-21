#ifndef RGBIMAGE_H
#define RGBIMAGE_H

#include <stdint.h>

#include <string>

class rgbimage {
 private:
  // Pointer to the buffer holding image data
  uint8_t *buffer;
  // Pointer to the buffer holding alpha channel data
  uint8_t *alpha;

  // Image width and height
  int w, h;
  int jpeg_compression;

 public:
  rgbimage();
  rgbimage(int w, int h);
  ~rgbimage();

  // Getters and setters
  int get_w() { return this->w; };
  int get_h() { return this->h; };
  void set_w(int w) { this->w = w; };
  void set_h(int h) { this->h = h; };
  uint8_t *getBuffer() { return this->buffer; };

  void setBuffer(uint8_t *buffer) { this->buffer = buffer; };
  void frombuffer(uint8_t *buffer, int w,
                  int h);  // Create an RGB image from a buffer

  // Transparency
  void addalpha(uint8_t value);
  void visible_to_alpha();
  void alpha_to_visible();
  uint8_t *getAlpha() { return alpha; };

  // Pixel manipulation
  uint8_t *getpixel(int x,
                    int y) const;  // Get the pixel value at a given position
  uint8_t *getpixel_inf(
      int x, int y) const;  // Get the pixel value at a given position, returns
                            // the top left pixel if outside of the image
  void putpixel(int x, int y,
                uint8_t *color);  // Set the pixel value at a given position
  void setcolor(uint8_t r, uint8_t g, uint8_t b);

  void paste(rgbimage *img, int x, int y);
  void paste2(rgbimage *img, int x, int y);

  // Convolution
  void convolve(rgbimage *kernel);
  rgbimage mkgaussiankernel(double stddev, double avg, int radius);

  // Filters
  void gaussianblur(double stdd);
  void expand(int radius);

  // Save
  void save_jpeg(std::string filename);  // Save an RGB image as a JPEG file
  void save_ppm(std::string filename);   // Save an RGB image as a PPM file
  void write_jpeg_mem(
      uint8_t **outbuffer,
      uint64_t *outbytes);  // Write an RGB image as a JPEG to a memory buffer

  void crop(int x0, int y0, int x1, int y1);
  void autocrop();
};

#endif // RGBIMAGE_H
