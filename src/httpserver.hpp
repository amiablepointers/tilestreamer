#ifndef TILESTREAMER_SRC_HTTPSERVER_H_
#define TILESTREAMER_SRC_HTTPSERVER_H_

#include <microhttpd.h>

#include <mutex>
#include <vector>
#include <iostream>
#include <cstring>

namespace tilestreamer {

class httpserver;

class streamsession {
 private:
  uint8_t *buffer;
  size_t buffersize;

  size_t offset;
  bool completed;

  httpserver *hs;

  volatile size_t frameno;

 public:
  streamsession();
  ~streamsession();

  void set_httpserver(httpserver *h);

  bool get_completed() { return completed; };
  void set_completed(bool val) { completed = val; };

  size_t get_offset() { return offset; };
  void set_offset(size_t o) { offset = o; };
  uint8_t *get_buffer() { return buffer; };
  void set_buffer(uint8_t *t) { buffer = t; };

  size_t get_buffersize() { return buffersize; };
  void set_buffersize(size_t s) { buffersize = s; };

  size_t get_frameno() { return frameno; };
  void set_frameno(size_t f) { frameno = f; };

  size_t get_readablebytes(size_t maxreadablebytes) {
    if (completed) {
      offset = buffersize;
      return 0;
    } else {
      if ((buffersize - offset) >= maxreadablebytes) {
        return maxreadablebytes;
      } else {
        return (buffersize - offset);
      }
    }
  };

  uint8_t *get_next_bufferptr() { return buffer + offset; };
  void set_readbytes(size_t bytesread) {
    offset = offset + get_readablebytes(bytesread);
    if ((offset > 0) && (offset >= buffersize)) {
      offset = buffersize;
      completed = true;
      //std::cout << "Frame " << frameno << " written" << std::endl;
    }
  }

  void update_frame();

  void print_info();
  httpserver *get_httpserver();
};

class httpserver {
 private:
  uint16_t port;

  struct MHD_Daemon *mhd_daemon;
  std::vector<streamsession *> sessions;

  uint8_t *buffer;
  size_t buffersize;

  std::mutex buffer_write_mutex;

  volatile size_t frameno;

 public:
  httpserver(uint16_t p);
  httpserver();
  ~httpserver();

  size_t get_frameno() { return frameno; };
  void set_frameno(size_t f) { frameno = f; };
  void set_port(uint16_t p) { port = p; };

  size_t get_buffersize() { return buffersize; };
  uint8_t *get_buffer() { return buffer; };

  void start();
  void stop();
  void update_frame();

  streamsession *new_session() {
    streamsession *temp = new streamsession();

    temp->set_httpserver(this);
    // Force the first frame
    temp->set_frameno(0);
    sessions.push_back(temp);

    return temp;
  };

  void print_info() {
    std::cout << "buffersize: " << buffersize << std::endl;
    std::cout << "sessions: " << this->sessions.size() << std::endl;
  }

  void write_lock() { buffer_write_mutex.lock(); }

  void write_unlock() { buffer_write_mutex.unlock(); }

  void update_frame(uint8_t *fbuffer, size_t bufsize) {
    buffer_write_mutex.lock();

    free(buffer);
    buffer = NULL;
    buffer = (uint8_t *)malloc(bufsize + 2048);

    sprintf((char *)buffer,
            "--myboundary\r\n"
            "Content-Type: image/jpeg\r\n"
            "Content-length: %lu\r\n\r\n",
            bufsize);
    size_t headerlen = strlen((char *)buffer);
    memcpy(buffer + headerlen, fbuffer, bufsize);
    buffersize = headerlen + bufsize + 2;
    buffer[buffersize - 2] = '\r';
    buffer[buffersize - 1] = '\n';
    frameno++;

    buffer_write_mutex.unlock();
  };
};

}

#endif  // TILESTREAMER_SRC_HTTPSERVER_H_
