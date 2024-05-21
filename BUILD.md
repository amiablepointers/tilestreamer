# Prerequisites

Ensure you have the following dependencies installed:

* FreeType2
* libgstreamer
* libjpeg
* libv4l2
* libinih
* libmicrohttpd
* libgtest

On Debian-based systems, you can install the build tools with:

```
apt-get install build-essential cmake
```

And the required dependencies with:

```
apt-get install libgstreamermm-1.0-dev libinih-dev libfreetype-dev libjpeg-dev libmicrohttpd-dev libgtest-dev
```

# Compiling

Clone the repository:

```
git clone https://github.com/amiablepointers/tilestreamer.git
cd tilestreamer
```

Create a build directory and navigate into it:

Build using cmake:
```
cmake . 
make
```
