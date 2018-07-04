set(SQSTDLIB_SRC sqstdaux.cpp
                 sqstdblob.cpp
                 sqstdio.cpp
                 sqstdmath.cpp
                 sqstdrex.cpp
                 sqstdstream.cpp
                 sqstdstring.cpp
                 sqstdsystem.cpp)

if(NOT DISABLE_DYNAMIC)
  add_library(sqstdlib SHARED ${SQSTDLIB_SRC})
  target_link_libraries(sqstdlib squirrel)
  if(NOT SQ_DISABLE_INSTALLER)
    install(TARGETS sqstdlib RUNTIME DESTINATION ${INSTALL_BIN_DIR}
                             LIBRARY DESTINATION ${INSTALL_LIB_DIR}
                             ARCHIVE DESTINATION ${INSTALL_LIB_DIR})
  endif()
endif()

if(NOT DISABLE_STATIC)
  add_library(sqstdlib_static STATIC ${SQSTDLIB_SRC})
  if(NOT SQ_DISABLE_INSTALLER)
    install(TARGETS sqstdlib_static ARCHIVE DESTINATION ${INSTALL_LIB_DIR})
  endif()
endif()

if(LONG_OUTPUT_NAMES)
  if(NOT DISABLE_DYNAMIC)
    set_target_properties(sqstdlib PROPERTIES OUTPUT_NAME sqstdlib3)
  endif()

  if(NOT DISABLE_STATIC)
    set_target_properties(sqstdlib_static PROPERTIES OUTPUT_NAME sqstdlib3_static)
  endif()
endif()
