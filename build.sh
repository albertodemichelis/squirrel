#!/bin/sh

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

if [ -d "build" ]; then
	cd build
	
	if [ -z "$1" ]; then
		echo "Building with Release Config."
		config="release"
	else
		echo "Building with $1 Config."
		config="$1"
	fi

	cmake --build . --config "$config"
	if [ $? -eq 0 ]; then
		echo "${GREEN}Libraries built and should be located in \"/build/bin/\" folder.${NC}"
	else
		echo "${RED}Error: Build failed.${NC}"
	fi

else
	echo "\"build\" folder does not exist. Call \"configure.sh\" to configure it."
fi
