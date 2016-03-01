#
# init
#

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# Disallow in-source builds. Note that you will still have to manually
# clean up a few files if you accidentally try an in-source build.
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES  ON)
if(",${CMAKE_SOURCE_DIR}," STREQUAL ",${CMAKE_BINARY_DIR},")
    message(FATAL_ERROR "ERROR: In-source builds are not allowed.")
endif()

project(squirrel C CXX)

#
# configuration options
#

option(ENABLE_STATIC "Build static library." ON)
if(WIN32 OR WIN64)
    set(ENABLE_SHARED OFF)
    option(ENABLE_UNICODE "Build with SQUNICODE." OFF)
else()
    option(ENABLE_SHARED "Build shared library." ON)
    set(ENABLE_UNICODE OFF)
endif()
option(ENABLE_WERROR "Build with -Werror." OFF)

if(NOT ENABLE_STATIC AND NOT ENABLE_SHARED)
    set(ENABLE_STATIC ON)
endif()
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

#
# targets
#

set(squirrel_SOURCES
    squirrel/sqapi.cpp
    squirrel/sqbaselib.cpp
    squirrel/sqclass.cpp
    squirrel/sqcompiler.cpp
    squirrel/sqdebug.cpp
    squirrel/sqfuncstate.cpp
    squirrel/sqlexer.cpp
    squirrel/sqmem.cpp
    squirrel/sqobject.cpp
    squirrel/sqstate.cpp
    squirrel/sqtable.cpp
    squirrel/sqvm.cpp
    sqstdlib/sqstdaux.cpp
    sqstdlib/sqstdblob.cpp
    sqstdlib/sqstdio.cpp
    sqstdlib/sqstdmath.cpp
    sqstdlib/sqstdrex.cpp
    sqstdlib/sqstdstream.cpp
    sqstdlib/sqstdstring.cpp
    sqstdlib/sqstdsystem.cpp
)

set(sq_SOURCES
    sq/sq.c
)

if(ENABLE_STATIC)
    add_library(squirrel_static STATIC ${squirrel_SOURCES})
    add_executable(sq_static ${sq_SOURCES})
    target_link_libraries(sq_static squirrel_static)
    if(1)
        set_target_properties(squirrel_static PROPERTIES OUTPUT_NAME squirrel)
        set_target_properties(sq_static       PROPERTIES OUTPUT_NAME sq)
    endif()
endif()
if(ENABLE_SHARED)
    add_library(squirrel_shared SHARED ${squirrel_SOURCES})
    add_executable(sq_shared ${sq_SOURCES})
    target_link_libraries(sq_shared squirrel_shared)
    set_target_properties(squirrel_shared PROPERTIES SOVERSION 3 VERSION 3.1.0)
    if(1 AND NOT ENABLE_STATIC)
        set_target_properties(squirrel_shared PROPERTIES OUTPUT_NAME squirrel)
        set_target_properties(sq_shared       PROPERTIES OUTPUT_NAME sq)
    endif()
endif()

#
# compilation flags
#

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(x "$ENV{CPPFLAGS}")
if(ENABLE_UNICODE)
    add_definitions(-DSQUNICODE)
endif()
if(CMAKE_C_COMPILER_ID MATCHES "^(AppleClang|Clang|GNU)$$")
    set(x "${x} -Wall -Wextra -Wcast-qual -Wpointer-arith -Wwrite-strings")
    # TODO: fix all -Wshadow warnings
    set(x "${x} -Wno-missing-field-initializers -Wno-unused-parameter")
    if(ENABLE_WERROR)
        set(x "${x} -Werror")
    endif()
    set(x "${x} -fno-strict-aliasing -fno-exceptions")
    if(1)
        set(x "${x} -fno-unwind-tables -fno-asynchronous-unwind-tables")
    endif()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()
if(CMAKE_C_COMPILER_ID MATCHES "^(MSVC)$$")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    set(x "${x} -W4 -wd4100 -wd4127")
    if(ENABLE_WERROR)
        set(x "${x} -WX")
    endif()
endif()
if(x MATCHES "[-0-9a-zA-Z]")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${x}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${x}")
endif()

if(1)
    message(STATUS "CMAKE_C_FLAGS=${CMAKE_C_FLAGS}")
    message(STATUS "CMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}")
endif()

#
# "make install"
#

if(NOT CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "" FORCE)
endif()
include(GNUInstallDirs)
if(ENABLE_STATIC)
    install(TARGETS sq_static       DESTINATION "${CMAKE_INSTALL_BINDIR}")
    install(TARGETS squirrel_static DESTINATION "${CMAKE_INSTALL_LIBDIR}")
endif()
if(ENABLE_SHARED)
    install(TARGETS sq_shared       DESTINATION "${CMAKE_INSTALL_BINDIR}")
    install(TARGETS squirrel_shared DESTINATION "${CMAKE_INSTALL_LIBDIR}")
    set_target_properties(sq_shared PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_FULL_LIBDIR}")
endif()
set(doc_DATA doc/sqstdlib3.chm doc/sqstdlib3.pdf doc/squirrel3.chm doc/squirrel3.pdf)
install(FILES ${doc_DATA} DESTINATION "${CMAKE_INSTALL_DOCDIR}")

# vim:set ft=cmake ts=4 sw=4 tw=0 et:
