set(SQUIRREL_SRC sqapi.cpp
                 sqbaselib.cpp
                 sqfuncstate.cpp
                 sqdebug.cpp
                 sqlexer.cpp
                 sqobject.cpp
                 sqcompiler.cpp
                 sqstate.cpp
                 sqtable.cpp
                 sqmem.cpp
                 sqvm.cpp
                 sqclass.cpp)

add_library(squirrel SHARED ${SQUIRREL_SRC})
install(TARGETS squirrel RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

add_library(squirrel_static STATIC ${SQUIRREL_SRC})
install(TARGETS squirrel_static ARCHIVE DESTINATION lib)
