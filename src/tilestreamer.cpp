#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <cassert>
#include <csignal>
#include <iostream>

#include "frameserver.hpp"
#include "httpserver.hpp"


const char *config_filename = NULL;
const char *out_dev = NULL;
tilestreamer::frameserver_config fsc;
tilestreamer::frameserver fs;
volatile int keep_alive = 1;
int port = -1;

void sig_handler(int signum) {
  std::cout << "SIGINT Received." << std::endl;
  std::cout << "Stopping streams..." << std::endl;
  keep_alive = 0;
  fs.stop_streams();
}

void print_usage(char *argv[]) {
  std::cerr << "Usage: " << argv[0] << " [options]" << std::endl;
  std::cerr << "Options:" << std::endl;
  std::cerr << "-f <filename>   Specify a configuration file" << std::endl;
  std::cerr << "-d <device>     Specify the device to connect to" << std::endl;
  std::cerr << "-p <port>       Specify the port to listen on" << std::endl;
  exit(EXIT_FAILURE);
}

int parse_opts(int argc, char *argv[]) {
  int opt;
  while ((opt = getopt(argc, argv, "f:d:p:")) != -1) {
    switch (opt) {
    case 'f': {
      config_filename = optarg;
    } break;
    case 'd': {
      out_dev = optarg;
    } break;
    case 'p': {
      port = atoi(optarg);
    } break;
    default: {
      print_usage(argv);
    } break;
    }
  }
  return 0;
}

void main_loop() {
  tilestreamer::httpserver httpd;
  uint8_t *jpegbuffer;
  unsigned long jpegbuffersize[1];

  v4ldev out_v4l;
  #ifndef __linux__
    out_v4l = NULL;
  #endif

  fs.start_streams();
  rgbimage *frame = fs.get_frame();

  if (port > 0) {
    jpegbuffer = (uint8_t *)malloc(frame->get_w() * frame->get_h() * 3);
    httpd.set_port(port);
    httpd.start();
  }

  if (out_dev != NULL) {
    out_v4l = v4ldev(out_dev, frame->get_w(), frame->get_h());
    out_v4l.open(out_dev, frame->get_w(), frame->get_h());
  }

  while (keep_alive) {
    fs.update_frame();

    if (port > 0) {
      jpegbuffersize[0] = frame->get_w() * frame->get_h() * 3;
      fs.get_frame_as_jpeg(jpegbuffer, jpegbuffersize);
      httpd.update_frame(jpegbuffer, jpegbuffersize[0]);
    }

    usleep(1000000 / fs.get_fps());

    if (out_dev != NULL) {
      out_v4l.write_frame(fs.get_frame());
    }
  }

  if (out_dev != NULL) {
    out_v4l.close();
  }

  fs.stop_streams();
  if (port > 0) {
    httpd.stop();
  }

  free(jpegbuffer);
  jpegbuffer = NULL;
  delete (frame);
}

int main(int argc, char *argv[]) {
  parse_opts(argc, argv);

  if (config_filename == NULL) {
    std::cerr << "Error: No config file specified, quitting." << std::endl;
    print_usage(argv);
    exit(EXIT_FAILURE);
  }

  if ((out_dev == NULL) && (port < 0)) {
    std::cerr << "Error: No output device or port specified, quitting."
              << std::endl;
    print_usage(argv);
    exit(EXIT_FAILURE);
  }

  std::signal(SIGINT, sig_handler);

  gst_init(&argc, &argv);

  fsc.read_ini_config(std::string(config_filename));
  fs.set_config(fsc);
  fsc.print();

  if (fsc.validate() == 0) {
    std::cout << "Configuration validation succeded, proceeding." << std::endl;
    main_loop();
  } else {
    std::cout << "Configuration validation failed, quitting." << std::endl;
    gst_deinit();
    exit(EXIT_FAILURE);
  }

  gst_deinit();
  exit(EXIT_SUCCESS);
}
