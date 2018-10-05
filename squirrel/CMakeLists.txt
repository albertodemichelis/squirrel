set(SQUIRREL_SRC sqapi.cpp
                 sqbaselib.cpp
                 sqclass.cpp
                 sqcompiler.cpp
                 sqdebug.cpp
                 sqfuncstate.cpp
                 sqlexer.cpp
                 sqmem.cpp
                 sqobject.cpp
                 sqstate.cpp
                 sqtable.cpp
                 sqvm.cpp)

if(NOT DISABLE_DYNAMIC)
  add_library(squirrel SHARED ${SQUIRREL_SRC})
  if(NOT SQ_DISABLE_INSTALLER)
    install(TARGETS squirrel RUNTIME DESTINATION ${INSTALL_BIN_DIR}
                         LIBRARY DESTINATION ${INSTALL_LIB_DIR}
                         ARCHIVE DESTINATION ${INSTALL_LIB_DIR})
  endif()
endif()

if(NOT DISABLE_STATIC)
  add_library(squirrel_static STATIC ${SQUIRREL_SRC})
  if(NOT SQ_DISABLE_INSTALLER)
    install(TARGETS squirrel_static ARCHIVE DESTINATION ${INSTALL_LIB_DIR})
  endif()
endif()

if(LONG_OUTPUT_NAMES)
  if(NOT DISABLE_DYNAMIC)
    set_target_properties(squirrel PROPERTIES OUTPUT_NAME squirrel3)
  endif()

  if(NOT DISABLE_STATIC)
    set_target_properties(squirrel_static PROPERTIES OUTPUT_NAME squirrel3_static)
  endif()
endif()
