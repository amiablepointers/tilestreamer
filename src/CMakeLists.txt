cmake_minimum_required(VERSION 3.10)

project(tilestreamer)

set(CMAKE_CXX_STANDARD 17)

include(CPack)

set(CPACK_PROJECT_NAME "tilestreamer")

find_package(PkgConfig)
pkg_check_modules (GSTREAMER gstreamer-1.0 REQUIRED)
if (UNIX AND NOT APPLE)
    pkg_check_modules (INIH inih REQUIRED)
endif()

if (WIN32)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "C:/msys64/mingw64/include")
    list(APPEND CMAKE_INCLUDE_DIR "C:/msys64/mingw64/include")
    include_directories("C:/msys64/mingw64/include")
    pkg_check_modules (INIH inih REQUIRED)
endif()

pkg_check_modules (FREETYPE freetype2 REQUIRED)
pkg_check_modules (LIBJPEG libjpeg REQUIRED)
pkg_check_modules (LIBPNG libpng REQUIRED)
pkg_check_modules (LIBMICROHTTPD libmicrohttpd REQUIRED)

add_executable(tilestreamer tilestreamer.cpp frameserver.cpp rgbimage.cpp stream.cpp textrenderer.cpp v4ldev.cpp httpserver.cpp )

target_include_directories(tilestreamer PUBLIC ${FREETYPE_INCLUDE_DIRS})
target_link_libraries(tilestreamer ${FREETYPE_LIBRARIES})

target_include_directories(tilestreamer PUBLIC ${GSTREAMER_INCLUDE_DIRS})
target_link_libraries(tilestreamer ${GSTREAMER_LIBRARIES})

target_include_directories(tilestreamer PUBLIC ${LIBJPEG_INCLUDE_DIRS})
target_link_libraries(tilestreamer ${LIBJPEG_LIBRARIES})

target_include_directories(tilestreamer PUBLIC ${LIBPNG_INCLUDE_DIRS})
target_link_libraries(tilestreamer ${LIBPNG_LIBRARIES})

target_include_directories(tilestreamer PUBLIC ${INIH_INCLUDE_DIRS})
target_link_libraries(tilestreamer ${INIH_LIBRARIES})

target_include_directories(tilestreamer PUBLIC ${LIBMICROHTTPD_INCLUDE_DIRS})
target_link_libraries(tilestreamer ${LIBMICROHTTPD_LIBRARIES})

target_link_libraries(tilestreamer -lm -lpthread)
