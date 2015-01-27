#!/bin/bash
mkdir -p ./container
cd ./container
mkdir -p ./bin
mkdir -p ./init
cd ../

cp ./init.so ../../../container/init/init

cd ./src/programs
	cd ./init
	make clean
	make
	cp ./init.so ../../../container/init/init
	cd ../

	cd  ./factor
	make clean
	make
	cp ./factor.so ../../../container/bin/factor
	cd ../
cd ../../

cd ./src/server
make clean
make
cp ./main ../../boot
cd ../../
