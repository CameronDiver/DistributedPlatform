#!/bin/bash
mkdir -p ./container
cd ./container
mkdir -p ./bin
mkdir -p ./sys
cd ../

cd ./src/programs
	cd ./init/src
	make clean
	make
	cd ../bin
	cp ./init ../../../../container/sys
	cd ../../

	cd  ./factor/src
	make clean
	make
	cd ../bin
	cp ./factor ../../../../container/bin
	cd ../../
	
	cd ./true/src
	make clean
	make
	cd ../bin
	cp ./true ../../../../container/bin
	cd ../../
	
	cd ./false/src
	make clean
	make
	cd ../bin
	cp ./false ../../../../container/bin
	cd ../../
	
	cd ./yes/src
	make clean
	make
	cd ../bin
	cp ./yes ../../../../container/bin
	cd ../../
	
	cd ./rabbits/src
	make clean
	make
	cd ../bin
	cp ./rabbits ../../../../container/bin
	cd ../../
	
	cd ./args/src
	make clean
	make
	cd ../bin
	cp ./args ../../../../container/bin
	cd ../../
cd ../../

cd ./src/server
make clean
make
cp ./main ../../boot
cd ../../
