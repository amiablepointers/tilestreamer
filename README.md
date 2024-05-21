# Tilestreamer
Tilestreamer is a video streaming server designed to combine multiple video streams into a unified grid display.\
It is built for contexts where there is a need to simultaneously monitor multiple video streams with low latency, such as in a security camera systems or a live events.

## Features
* The server configuration is done using INI files, parsed with [inih](https://github.com/benhoyt/inih)
* Supports, using gstreamer, H.264 (via rtsp) and MJPEG (via http) video formats
* Supports overlay text for each tile with [FreeType2](https://freetype.org/)
* The output can be used with any V4L-compatible application
* Output can be streamed directly to a web browser as JPEG stream using [GNU Libmicrohttpd](https://www.gnu.org/software/libmicrohttpd/)

## Building the Project
For instructions on how to build the project from source, please refer to the [BUILD](BUILD.md) file.

## Configuration
For a guide on configuring Tilestreamer, please refer to the [CONFIGURATION](CONFIGURATION.md) file.

## License
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
