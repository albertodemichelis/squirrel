if(MSVC)
  cmake_minimum_required(VERSION 3.4)
else()
  cmake_minimum_required(VERSION 2.8)
endif()

option(DISABLE_STATIC "Avoid building/installing static libraries.")
option(LONG_OUTPUT_NAMES "Use longer names for binaries and libraries: squirrel3 (not sq).")

set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}" CACHE PATH "")
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif ()

project(squirrel C CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

if(CMAKE_COMPILER_IS_GNUCXX)
  set(SQ_FLAGS -fno-exceptions -fno-strict-aliasing -Wall -Wextra -pedantic -Wcast-qual)

  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(SQ_FLAGS ${SQ_FLAGS} -O3)
  elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set(SQ_FLAGS ${SQ_FLAGS} -O3 -g)
  elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    set(SQ_FLAGS ${SQ_FLAGS} -Os)
  elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(SQ_FLAGS ${SQ_FLAGS} -pg -pie -gstabs -g3 -Og)
  endif()

  if(CMAKE_VERSION VERSION_GREATER 3)
    add_compile_options(${SQ_FLAGS})
  else()
    add_definitions(${SQ_FLAGS})
  endif()

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -std=c++0x")
elseif(MSVC)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  add_definitions(-D_SQ64)
endif()

if(NOT SQ_DISABLE_INSTALLER)
  if(NOT INSTALL_BIN_DIR)
    set(INSTALL_BIN_DIR bin)
  endif()

  if(NOT INSTALL_LIB_DIR)
    set(INSTALL_LIB_DIR lib)
  endif()
endif()

add_subdirectory(squirrel)
add_subdirectory(sqstdlib)
add_subdirectory(sq)

if(NOT WIN32 AND NOT DISABLE_DYNAMIC)
  set_target_properties(squirrel sqstdlib PROPERTIES SOVERSION 0 VERSION 0.0.0)
endif()

if(INSTALL_INC_DIR)
  set(SQ_PUB_HEADERS include/sqconfig.h
                     include/sqstdaux.h
                     include/sqstdblob.h
                     include/sqstdio.h
                     include/sqstdmath.h
                     include/sqstdstring.h
                     include/sqstdsystem.h
                     include/squirrel.h)
  install(FILES ${SQ_PUB_HEADERS} DESTINATION ${INSTALL_INC_DIR})
endif()
