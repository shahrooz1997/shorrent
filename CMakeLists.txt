cmake_minimum_required(VERSION 3.17)
project(shorrent)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -Wall -Werror -Wconversion")

add_custom_command(OUTPUT gbuffer.pb.cc gbuffer.pb.h
        COMMAND protoc ARGS --proto_path=${CMAKE_SOURCE_DIR} --cpp_out=${CMAKE_BINARY_DIR} gbuffer.proto
        DEPENDS gbuffer.proto)

add_executable(Peer peer_driver.cpp FileHandler.cpp File.cpp Chunk.cpp Util.cpp Server.cpp Peer.cpp
        ${CMAKE_BINARY_DIR}/gbuffer.pb.cc)

add_executable(Server server_driver.cpp FileHandler.cpp File.cpp Chunk.cpp Util.cpp Server.cpp Peer.cpp
        ${CMAKE_BINARY_DIR}/gbuffer.pb.cc)


target_link_libraries(Peer pthread protobuf)
target_link_libraries(Server pthread protobuf)

include_directories(${CMAKE_BINARY_DIR})
