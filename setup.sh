#!/bin/bash
mkdir ./container
mkdir ./container/init

cp ./init.so ../../../container/init/init
cd ./src/programs/init
make clean
make
cd ../../../

cd ./src/server
make clean
make
cp ./main ../../boot
cd ../../
