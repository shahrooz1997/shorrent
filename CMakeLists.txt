cmake_minimum_required(VERSION 3.17)
project(shorrent)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Werror -Wconversion")

add_executable(shorrent main.cpp FileHandler.cpp FileHandler.h File.cpp File.h Chunk.cpp Chunk.h Util.cpp Util.h Server.cpp Server.h gbuffer.pb.cc gbuffer.pb.h)

add_executable(Server server_driver.cpp FileHandler.cpp FileHandler.h File.cpp File.h Chunk.cpp Chunk.h Util.cpp Util.h Server.cpp Server.h gbuffer.pb.cc gbuffer.pb.h Peer.cpp Peer.h)


TARGET_LINK_LIBRARIES(shorrent pthread protobuf)
TARGET_LINK_LIBRARIES(Server pthread protobuf)
