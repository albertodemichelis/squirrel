cmake_minimum_required(VERSION 2.8)
project(sqstd)

# sources
set(SQSTD_SRCS
    sqstdblob.cpp
    sqstdio.cpp
    sqstdstream.cpp
    sqstdmath.cpp
    sqstdsystem.cpp
    sqstdstring.cpp 
    sqstdaux.cpp
    sqstdrex.cpp
)

# shared lib
add_library(sqstd SHARED ${SQSTD_SRCS})

# static lib
add_library(sqstd_static STATIC ${SQSTD_SRCS})
