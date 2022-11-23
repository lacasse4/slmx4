cmake_minimum_required(VERSION 3.1)

project(main)
set (CMAKE_CXX_STANDARD 11)

SET(EXECUTABLE_OUTPUT_PATH ./bin)
  
set(CMAKE_CXX_FLAGS "-g -Wall")
set(CMAKE_C_FLAGS "-g -Wall")

include_directories(./inc)

add_executable(main src/main.cpp src/slmx4_vcom.cpp src/serialib.cpp)

INSTALL(TARGETS main RUNTIME DESTINATION bin)

