set(SQSTDLIB_SRC sqstdaux.cpp
                 sqstdblob.cpp
                 sqstdio.cpp
                 sqstdmath.cpp
                 sqstdrex.cpp
                 sqstdstream.cpp
                 sqstdstring.cpp
                 sqstdsystem.cpp)

add_library(sqstdlib SHARED ${SQSTDLIB_SRC})
target_link_libraries(sqstdlib squirrel)
install(TARGETS sqstdlib RUNTIME DESTINATION ${INSTALL_BIN_DIR}
                         LIBRARY DESTINATION ${INSTALL_LIB_DIR}
                         ARCHIVE DESTINATION ${INSTALL_LIB_DIR})

if(NOT DEFINED DISABLE_STATIC)
  add_library(sqstdlib_static STATIC ${SQSTDLIB_SRC})
  install(TARGETS sqstdlib_static ARCHIVE DESTINATION ${INSTALL_LIB_DIR})
endif()

if(DEFINED LONG_OUTPUT_NAMES)
  set_target_properties(sqstdlib PROPERTIES OUTPUT_NAME sqstdlib3)

  if(NOT DEFINED DISABLE_STATIC)
    set_target_properties(sqstdlib_static PROPERTIES OUTPUT_NAME sqstdlib3_static)
  endif()
endif()
