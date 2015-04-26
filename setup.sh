#!/bin/bash

# Make directory structure.
mkdir -p ./container
cd ./container
	mkdir -p ./bin
	mkdir -p ./home
	mkdir -p ./sys
	mkdir -p ./dev
cd ../

# Create device files.
cd ./container/dev
	touch ./null
	touch ./zero
	touch ./full
cd ../../

# Copy default profile script.
cd ./src/misc
	cp profile ../../container/home/.profile
cd ../../

# Copy programs.
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

	cd ./shell/src
	make clean
	make
	cd ../bin
	cp ./shell ../../../../container/bin
	cd ../../

	cd ./printenv/src
	make clean
	make
	cd ../bin
	cp ./printenv ../../../../container/bin
	cd ../../

	cd ./cat/src
	make clean
	make
	cd ../bin
	cp ./cat ../../../../container/bin
	cd ../../
cd ../../

# Build server.
cd ./src/server
make clean
make
cp ./main ../../server
cd ../../
