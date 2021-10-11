#!/bin/bash

mkdir cmake-build-install
cd cmake-build-install
cmake ../
make -j4
mv Server ../
mv Peer ../
