if(MSVC)
  cmake_minimum_required(VERSION 3.4)
else()
  cmake_minimum_required(VERSION 2.8)
endif()

set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}" CACHE PATH "")
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "")

project(squirrel C CXX)

include_directories(${CMAKE_SOURCE_DIR}/include)

if(CMAKE_COMPILER_IS_GNUCXX)
  set(SQ_FLAGS -fno-exceptions -fno-strict-aliasing -Wall -Wextra -pedantic -Wcast-qual)

  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(SQ_FLAGS ${SQ_FLAGS} -O3)
  else()
    set(SQ_FLAGS ${SQ_FLAGS} -pg -pie -gstabs -g3)
  endif()

  if(CMAKE_VERSION VERSION_GREATER 3)
    add_compile_options(${SQ_FLAGS})
  else()
    add_definitions(${SQ_FLAGS})
  endif()

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -std=c++0x")
elseif(MSVC)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  add_definitions(-D_SQ64)
endif()

add_subdirectory(squirrel)
add_subdirectory(sqstdlib)
add_subdirectory(sq)

install(FILES doc/squirrel3.pdf doc/sqstdlib3.pdf DESTINATION share/doc/squirrel3)
