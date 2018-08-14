cmake_minimum_required(VERSION 3.7)
project(AirportSimulator)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp parking_stand.cpp parking_stand.h runway.cpp runway.h airport.cpp airport.h tokens.h)
add_executable(AirportSimulator ${SOURCE_FILES})