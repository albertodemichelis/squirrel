# Let the script find the documentation
include_directories(../doc/source)

if(NOT DEFINED DISABLE_DYNAMIC)
  if(CMAKE_COMPILER_IS_GNUCXX)
    set_source_files_properties(build_documentation.c PROPERTIES COMPILE_FLAGS -std=c99)
  endif()
  add_executable(sqtestdoc build_documentation.c)
  set_target_properties(sqtestdoc PROPERTIES LINKER_LANGUAGE C)
  target_link_libraries(sqtestdoc squirrel sqstdlib)
endif()

if(NOT DEFINED DISABLE_STATIC)
  add_executable(sqtestdoc_static build_documentation.c)
  set_target_properties(sqtestdoc_static PROPERTIES LINKER_LANGUAGE C)
  target_link_libraries(sqtestdoc_static squirrel_static sqstdlib_static)
endif()

if(CMAKE_COMPILER_IS_GNUCXX AND (NOT DEFINED DISABLE_STATIC))
  set_target_properties(sqtestdoc_static PROPERTIES COMPILE_FLAGS "-static -Wl,-static")
endif()
