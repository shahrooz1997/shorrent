#!/bin/bash

mkdir cmake-build-install > /dev/null 2>&1
set -e
#protoc --cpp_out=./ gbuffer.proto
cd cmake-build-install
cmake ../
make -j4
mv Server ../
mv Peer ../
