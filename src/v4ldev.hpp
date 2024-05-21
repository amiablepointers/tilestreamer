#ifndef V4LDEV_H
#define V4LDEV_H

#include "rgbimage.hpp"

#ifdef __linux__

class v4ldev {
  private:
    std::string dev;
    uint16_t w, h;
    int fd;
    bool isopen;

  public:
    v4ldev();
    v4ldev(std::string dev, uint16_t w, uint16_t h);

    void open(std::string dev, uint16_t w, uint16_t h);
    void close();
    void write_frame(rgbimage *img);
    void write_jpeg_frame(uint8_t *buffer, size_t length);

    bool is_open();
};

#endif

#endif
