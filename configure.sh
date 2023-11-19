#!/bin/sh

mkdir build -p

if [ -d "build" ]; then
	cd build 
	cmake .. $*
else 
	echo "Unable to create \"build\" folder. Stopping configuration. \n"
fi
