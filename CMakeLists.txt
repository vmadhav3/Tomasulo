cmake_minimum_required(VERSION 3.3)
project(cpe5110)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES parse.cpp)
add_executable(cpe5110 ${SOURCE_FILES})