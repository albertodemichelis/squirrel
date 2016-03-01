#! /bin/bash
## vim:set ts=4 sw=4 et:
set -e; set -o pipefail

echo "Checking source code for whitespace violations"
find . \
    -type d -name .git  -prune -o \
    -type d -name build -prune -o \
    -type f -iname '*.chm' -prune -o \
    -type f -iname '*.dsp' -prune -o \
    -type f -iname '*.dsw' -prune -o \
    -type f -iname '*.pdf' -prune -o \
    -type f -print0 | LC_ALL=C sort -z | \
xargs -0r perl -n -e '
    if (m,[\r\x1a],) { print "ERROR: DOS EOL detected $ARGV: $_"; exit(1); }
    if (m,[ \t]$,) { print "ERROR: trailing whitespace detected $ARGV: $_"; exit(1); }
    if (m,\t, && $ARGV !~ m,makefile,i) { print "ERROR: hard TAB detected $ARGV: $_"; exit(1); }
' || exit 1

echo "BUILD_METHOD_AND_BUILD_TYPE='$BUILD_METHOD_AND_BUILD_TYPE'"
echo "VERBOSE='$VERBOSE'"
echo "CC='$CC'"
echo "CXX='$CXX'"
echo "CPPFLAGS='$CPPFLAGS'"
echo "CFLAGS='$CFLAGS'"
echo "CXXFLAGS='$CXXFLAGS'"
echo "LDFLAGS='$LDFLAGS'"
echo "BUILD_DIR='$BUILD_DIR'"

mkdir -p "$BUILD_DIR" || exit 1
cd "$BUILD_DIR" || exit 1

set -x
cmake --version
pwd
#env | LC_ALL=C sort

if ! test -f "$TRAVIS_BUILD_DIR/CMakeLists.txt"; then
    echo "ERROR: invalid TRAVIS_BUILD_DIR '$TRAVIS_BUILD_DIR'"
    exit 1
fi

case $BUILD_METHOD_AND_BUILD_TYPE in
cmake/debug)
    cmake "$TRAVIS_BUILD_DIR" -DCMAKE_BUILD_TYPE=Debug   "-DCMAKE_INSTALL_PREFIX=$PWD/install" -DENABLE_WERROR=1
    make VERBOSE=$VERBOSE
    make VERBOSE=$VERBOSE install
    ;;
cmake/release)
    cmake "$TRAVIS_BUILD_DIR" -DCMAKE_BUILD_TYPE=Release "-DCMAKE_INSTALL_PREFIX=$PWD/install" -DENABLE_WERROR=1
    make VERBOSE=$VERBOSE
    make VERBOSE=$VERBOSE install
    ;;
*)
    echo "ERROR: invalid build '$BUILD_METHOD_AND_BUILD_TYPE'"
    exit 1
    ;;
esac
