#!/bin/bash

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
cd ..
mkdir peer2
cd peer2
ln -s "${app_path}/Peer" ./Peer
cd ..

cp -r "${app_path}/../files" ./peer1/
cp -r "${app_path}/../files" ./peer2/
cp -r "${app_path}/../chunks" ./peer1/
cp -r "${app_path}/../chunks" ./peer2/

cp "${app_path}/Server" ./
cd test
cd peer1
gnome-terminal
cd ../peer2
rm -rf files/* chunks/*
gnome-terminal
cd ..
./Server
