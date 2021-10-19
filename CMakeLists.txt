cmake_minimum_required(VERSION 3.10)
project(shorrent)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -Wall -Werror")

file(GLOB SOURCES *.cpp)
set(LIB_SOURCES ${SOURCES})
set(MAIN_SOURCES ${CMAKE_SOURCE_DIR}/server_driver.cpp ${CMAKE_SOURCE_DIR}/peer_driver.cpp)
list(REMOVE_ITEM LIB_SOURCES ${MAIN_SOURCES})

#add_custom_target(gbuffer
#        protoc --proto_path=${CMAKE_SOURCE_DIR} --cpp_out=${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/gbuffer.proto
#        BYPRODUCTS ${CMAKE_BINARY_DIR}/gbuffer.pb.cc ${CMAKE_BINARY_DIR}/gbuffer.pb.h
#        DEPENDS ${CMAKE_SOURCE_DIR}/gbuffer.proto)

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/gbuffer.pb.cc ${CMAKE_BINARY_DIR}/gbuffer.pb.h
        COMMAND protoc ARGS --proto_path=${CMAKE_SOURCE_DIR} --cpp_out=${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/gbuffer.proto
        DEPENDS ${CMAKE_SOURCE_DIR}/gbuffer.proto)

link_libraries(pthread protobuf)
include_directories(${CMAKE_BINARY_DIR})
add_library(shorrent_lib ${LIB_SOURCES} ${CMAKE_BINARY_DIR}/gbuffer.pb.cc)

link_libraries(shorrent_lib)
add_executable(Peer ${CMAKE_SOURCE_DIR}/peer_driver.cpp)
add_executable(Server ${CMAKE_SOURCE_DIR}/server_driver.cpp)
