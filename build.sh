#!/bin/sh

if [ -d "build" ]; then
	cd build
	cmake --build . --config $1
	echo "Libraries builded and should be located in \"/build/bin/\" folder. \n"
else 
	echo "\"build\" folder does not exist. Call \"configure.sh\" to configure it. \n"
fi
