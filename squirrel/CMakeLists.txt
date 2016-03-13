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

add_library(squirrel SHARED ${SQUIRREL_SRC})
install(TARGETS squirrel RUNTIME DESTINATION ${INSTALL_BIN_DIR}
                         LIBRARY DESTINATION ${INSTALL_LIB_DIR}
                         ARCHIVE DESTINATION ${INSTALL_LIB_DIR})

add_library(squirrel_static STATIC ${SQUIRREL_SRC})
install(TARGETS squirrel_static ARCHIVE DESTINATION ${INSTALL_LIB_DIR})
