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
	
	cd ./true
	make clean
	make
	cp ./true.so ../../../container/bin/true
	cd ../
	
	cd ./false
	make clean
	make
	cp ./false.so ../../../container/bin/false
	cd ../
	
	cd ./yes
	make clean
	make
	cp ./yes.so ../../../container/bin/yes
	cd ../
	
	cd ./rabbits
	make clean
	make
	cp ./rabbits.so ../../../container/bin/rabbits
	cd ../
cd ../../

cd ./src/server
make clean
make
cp ./main ../../boot
cd ../../
