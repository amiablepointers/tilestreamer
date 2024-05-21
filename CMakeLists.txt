cmake_minimum_required(VERSION 3.10)

if (UNIX AND NOT APPLE)
    set(CMAKE_C_COMPILER /usr/bin/gcc)
    set(CMAKE_CXX_COMPILER /usr/bin/g++)
    set(CMAKE_C_FLAGS "-g -Wall")
    set(CMAKE_CXX_FLAGS "-g -Wall")
endif()

if (WIN32)
    set(CMAKE_C_COMPILER C:/msys64/usr/bin/gcc.exe)
    set(CMAKE_CXX_COMPILER C:/msys64/usr/bin/g++.exe)
    set(CMAKE_C_FLAGS "-g -Wall")
    set(CMAKE_CXX_FLAGS "-g -Wall")
endif()

project(tilestreamer)

add_subdirectory(src)
add_subdirectory(tests)
