#!/bin/sh

if [ -d "build" ]; then
	cd build
	make
	echo "Libraries builded and should be located in \"/build/bin/\" folder. \n"
else 
	echo "\"build\" folder does not exist."
fi
