#!/bin/bash

set -e

rm -rf test
cd cmake-build-debug
cmake ../
make clean
make -j8
app_path=`pwd`
cd ..

mkdir test
cd test
mkdir peer1
cd peer1
ln -s "${app_path}/Peer" ./Peer
cp -r "${app_path}/../files" ./
mkdir chunks
# cp -r "${app_path}/../chunks" ./
gnome-terminal
cd ..

mkdir peer2
cd peer2
ln -s "${app_path}/Peer" ./Peer
mkdir files
mkdir chunks
# cp -r "${app_path}/../files" ./
# cp -r "${app_path}/../chunks" ./
gnome-terminal
cd ..

mkdir peer3
cd peer3
ln -s "${app_path}/Peer" ./Peer
mkdir files
mkdir chunks
# cp -r "${app_path}/../files" ./
# cp -r "${app_path}/../chunks" ./
gnome-terminal
cd ..

cp "${app_path}/Server" ./Server
./Server
