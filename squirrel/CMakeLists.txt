cmake_minimum_required(VERSION 2.8)
project(libsquirrel)

# sources
set(SQUIRREL_SRCS
    sqapi.cpp
    sqbaselib.cpp
    sqfuncstate.cpp
    sqdebug.cpp
    sqlexer.cpp
    sqobject.cpp
    sqcompiler.cpp
    sqstate.cpp
    sqtable.cpp
    sqmem.cpp
    sqvm.cpp
    sqclass.cpp
)

# shared lib
add_library(squirrel SHARED ${SQUIRREL_SRCS})

# static lib
add_library(squirrel_static STATIC ${SQUIRREL_SRCS})
