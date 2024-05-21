cmake_minimum_required(VERSION 3.10)

project(tilestreamer)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_COMPILER /usr/bin/g++)
set(CMAKE_CXX_FLAGS "-g -Wall -Werror")

find_package(PkgConfig)
pkg_check_modules (GSTREAMER gstreamer-1.0 REQUIRED)
pkg_check_modules (INIH inih REQUIRED)
pkg_check_modules (FREETYPE freetype2 REQUIRED)
pkg_check_modules (LIBJPEG libjpeg REQUIRED)
pkg_check_modules (LIBPNG libpng REQUIRED)
pkg_check_modules (GTEST gtest REQUIRED)
pkg_check_modules (LIBMICROHTTPD libmicrohttpd REQUIRED)

add_executable(tests 
    tests.cpp 
    ../src/frameserver.hpp 
    ../src/rgbimage.hpp 
    ../src/stream.hpp 
    ../src/textrenderer.hpp 
    ../src/v4ldev.hpp

    ../src/frameserver.cpp 
    ../src/httpserver.cpp 
    ../src/rgbimage.cpp 
    ../src/stream.cpp 
    ../src/textrenderer.cpp 
    ../src/v4ldev.cpp)

target_include_directories(tests PUBLIC ../src)

target_include_directories(tests PUBLIC ${FREETYPE_INCLUDE_DIRS})
target_link_libraries(tests ${FREETYPE_LIBRARIES})

target_include_directories(tests PUBLIC ${GSTREAMER_INCLUDE_DIRS})
target_link_libraries(tests ${GSTREAMER_LIBRARIES})

target_include_directories(tests PUBLIC ${LIBJPEG_INCLUDE_DIRS})
target_link_libraries(tests ${LIBJPEG_LIBRARIES})

target_include_directories(tests PUBLIC ${LIBPNG_INCLUDE_DIRS})
target_link_libraries(tests ${LIBPNG_LIBRARIES})

target_include_directories(tests PUBLIC ${INIH_INCLUDE_DIRS})
target_link_libraries(tests ${INIH_LIBRARIES})

target_include_directories(tests PUBLIC ${GTEST_INCLUDE_DIRS})
target_link_libraries(tests ${GTEST_LIBRARIES})

target_include_directories(tests PUBLIC ${LIBMICROHTTPD_INCLUDE_DIRS})
target_link_libraries(tests ${LIBMICROHTTPD_LIBRARIES})

target_link_libraries(tests  -lm -lpthread)

