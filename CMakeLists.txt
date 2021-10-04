cmake_minimum_required(VERSION 3.17)
project(shorrent)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Werror -Wconversion")

add_executable(shorrent main.cpp Server.cpp Server.h File.cpp File.h Chunk.cpp Chunk.h Util.cpp Util.h)

TARGET_LINK_LIBRARIES(shorrent pthread)
