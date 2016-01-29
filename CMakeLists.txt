cmake_minimum_required(VERSION 2.8)
project(squirrel)

# get machine
execute_process( COMMAND ${CMAKE_C_COMPILER} -dumpmachine OUTPUT_VARIABLE DUMP_MACHINE OUTPUT_STRIP_TRAILING_WHITESPACE)
message("Dump Machine       : ${DUMP_MACHINE}")

# get architecture
if(NOT DEFINED ARCHITECTURE)
  string(FIND ${DUMP_MACHINE} "-" DUMP_MACHINE_STRIP)
  string(SUBSTRING ${DUMP_MACHINE} 0 ${DUMP_MACHINE_STRIP} ARCHITECTURE)
endif()
message("Architecture       : ${ARCHITECTURE}")

# global includes
include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

# global flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti -Wall -fno-strict-aliasing")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-exceptions -Wall -fno-strict-aliasing")

set(SQGLOB_FLAGS_RELEASE "-O2")
set(SQGLOB_FLAGS_DEBUG "-pg -pie -gstabs -g3")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${SQGLOB_FLAGS_RELEASE}")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_DEBUG} ${SQGLOB_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${SQGLOB_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_DEBUG} ${SQGLOB_FLAGS_DEBUG}")

if("${ARCHITECTURE}" STREQUAL "x86_64")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_SQ64")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_SQ64")
endif()

# subdirectories
add_subdirectory(squirrel)
add_subdirectory(sqstdlib)
add_subdirectory(sq)
