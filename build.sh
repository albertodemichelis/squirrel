#!/bin/sh

if [ -d "build" ]; then
	cd build
	if [ -z "$1" ]; then
		echo Building with Release Config.
		cmake --build . --config release
	else
		echo Building with $1 Config.
		cmake --build . --config $1
	fi

	echo "Libraries builded and should be located in \"/build/bin/\" folder. \n"
else 
	echo "\"build\" folder does not exist. Call \"configure.sh\" to configure it. \n"
fi
