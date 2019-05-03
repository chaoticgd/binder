cmake_minimum_required (VERSION 2.8.11)
project(binder)
set(CMAKE_REQUIRED_FLAGS -std=c++17)
add_executable(bin/binder src/binder.cpp)
