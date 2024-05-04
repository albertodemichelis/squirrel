#!/bin/sh

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

TITLE="Squirrel-lang installer"
PROGNAME=$(basename $0)
RELEASE="Revision 1.0"
INSTALL=0


print_release() {
    echo "$RELEASE"
}

print_usage() {
        echo ""
        echo "$TITLE"
        echo ""
        echo "Usage: ./$PROGNAME | [-h | --help] | [-v | --version] | [-i | --install]"
        echo ""
		echo "          -h, --help		Help message"
		echo "          -v, --version		Version"
        echo "          -i, --install		Install all binaries on your computer (in /bin)"
		echo "          -u, --uninstall		Big mistake (uninstall Squirrel-lang)"
        echo ""
}

print_help() {
        echo ""
        print_usage
        echo ""
        echo ""
		print_release $PROGNAME $RELEASE
		exit 0
}

uninstall(){
	sudo rm /bin/sq /bin/sq_static
	echo "${GREEN}Squirrel uninstalled${NC}"
	echo "👋 Goodbye 👋"
	exit 0
}

while [ $# -gt 0 ]; do
    case "$1" in
        -h | --help)
            print_help
            exit 0
            ;;
        -v | --version)
                print_release
                exit 0
                ;;
        -i | --install)
                INSTALL=1
                ;;
		-u | --uninstall)
                uninstall
                ;;
        *)  echo "Unknown argument: $1"
            print_usage
            ;;
        esac
shift
done



if [ -d "build" ]; then
	if [ -z "$1" ]; then
		echo "Building with Release Config."
		config="release"
	else
		echo "Building with $1 Config."
		config="$1"
	fi
else
	mkdir build -p

	if [ -d "build" ]; then
		cd build 
		cmake .. $*
	else 
		echo "${RED}Unable to create \"build\" folder. Stopping configuration. \n${NC}"
	fi
fi

cd build

cmake --build . --config "$config"
if [ $? -eq 0 ]; then
	echo "${GREEN}Libraries built and should be located in \"/build/bin/\" folder.${NC}"
	if [ $INSTALL  -eq 1 ]; then
		sudo cp ./bin/sq /bin/sq
		sudo cp ./bin/sq_static /bin/sq_static
		echo "${GREEN}Libraries installed, feel free to try sq or sq_static.${NC}"
	fi
else
	echo "${RED}Error: Build failed.${NC}"
fi