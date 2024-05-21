#include "httpserver.hpp"

#include <microhttpd.h>
#include <string.h>

#include <iostream>
#include <string>
#include <vector>

namespace tilestreamer {

httpserver::httpserver() {
  buffer = NULL;
  buffersize = 0;
  frameno = 1;
  port = 8099;
}

httpserver::httpserver(uint16_t p) {
  buffer = NULL;
  buffersize = 0;
  frameno = 1;
  port = p;
}

httpserver::~httpserver() {
  free(buffer);
  buffer = NULL;
}

static ssize_t data_generator(void *cls, uint64_t pos, char *buf, size_t max) {
  streamsession *s = (streamsession *)cls;
  size_t maxreadable = s->get_readablebytes(max);

  if (!s->get_completed()) {
    // std::cout << "Write Frame..." << std::endl;
    memcpy(buf, s->get_next_bufferptr(), maxreadable);
    s->set_readbytes(maxreadable);
  } else {
    // Update frame if new available
    s->update_frame();
  }
  return maxreadable;
}

static MHD_Result handle_request(void *cls, struct MHD_Connection *connection,
                                 const char *url, const char *method,
                                 const char *version, const char *upload_data,
                                 size_t *upload_data_size, void **ptr) {
  httpserver *httpd = (httpserver *)cls;
  streamsession *s = httpd->new_session();

  s->print_info();
  httpd->print_info();

  struct MHD_Response *response = MHD_create_response_from_callback(
      MHD_SIZE_UNKNOWN, 1024, &data_generator, s, NULL);

  httpd->write_unlock();

  if (response == NULL) {
    return MHD_NO;
  }

  // Add the appropriate headers
  MHD_add_response_header(
      response, "Cache-Control",
      "no-store, no-cache, must-revalidate, proxy-revalidate, pre-check=0, "
      "post-check=0, max-age=0");
  MHD_add_response_header(response, "Pragma", "no-cache");
  MHD_add_response_header(response, "Content-Type",
                          "multipart/x-mixed-replace; boundary=myboundary");

  // Send the response
  MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
  MHD_destroy_response(response);
  httpd->write_unlock();

  return ret;
}

void httpserver::start() {
  mhd_daemon = MHD_start_daemon(
      MHD_USE_THREAD_PER_CONNECTION, port, NULL, NULL, &handle_request, this,
      MHD_OPTION_CONNECTION_TIMEOUT, 2, MHD_OPTION_END);

  if (mhd_daemon == NULL) {
    std::cout << "MicroHTTPd could NOT be started." << std::endl;
  } else {
    std::cout << "MicroHTTPd started on ";
    std::cout << "http://localhost:" << port << std::endl;
  }
}

void httpserver::stop() {
  write_unlock();
  MHD_stop_daemon(mhd_daemon);
  std::cout << "MicroHTTPd stopped." << std::endl;
}

streamsession::streamsession() {
  buffer = (uint8_t *)malloc(10000000);
  offset = 0;
  buffersize = 0;
  completed = true;
  frameno = 0;
}

streamsession::~streamsession() {
  free(buffer);
  buffer = NULL;
  offset = 0;
  buffersize = 0;
  frameno = 0;
}

void streamsession::update_frame() {
  while (!(frameno < hs->get_frameno())) {
    usleep(1000);
  }
  hs->write_lock();
  memcpy(buffer, hs->get_buffer(), hs->get_buffersize());
  buffersize = hs->get_buffersize();
  frameno = hs->get_frameno();
  completed = false;
  offset = 0;
  hs->write_unlock();
}

void streamsession::set_httpserver(httpserver *h) { hs = h; }

void streamsession::print_info() {
  std::cout << "buffersize: " << hs->get_buffersize() << std::endl;
  std::cout << "offset: " << offset << std::endl;
}

httpserver *streamsession::get_httpserver() { return hs; }

}
