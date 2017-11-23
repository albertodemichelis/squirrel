# Let the code find the documentation sources
include_directories(../doc/source)

set(SOURCES
	build_documentation.cpp
	reference_embedding.cpp
)

if(NOT DEFINED DISABLE_DYNAMIC)
  add_executable(sqtestdoc ${SOURCES})
  set_target_properties(sqtestdoc PROPERTIES LINKER_LANGUAGE C)
  target_link_libraries(sqtestdoc squirrel sqstdlib)
endif()

if(NOT DEFINED DISABLE_STATIC)
  add_executable(sqtestdoc_static ${SOURCES})
  target_link_libraries(sqtestdoc_static squirrel_static sqstdlib_static)
endif()

if(CMAKE_COMPILER_IS_GNUCXX AND (NOT DEFINED DISABLE_STATIC))
  set_target_properties(sqtestdoc_static PROPERTIES COMPILE_FLAGS "-static -Wl,-static")
endif()
