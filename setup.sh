#!/bin/bash
mkdir ./container
mkdir ./container/init

cd ./src/programs/init
make clean
make
cp ./init.so ../../../container/init/init.so
cd ../../../

cd ./src/server
make clean
make
cp ./main ../../boot
cd ../../
