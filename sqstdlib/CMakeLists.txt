set(SQSTDLIB_SRC sqstdblob.cpp
                 sqstdio.cpp
                 sqstdstream.cpp
                 sqstdmath.cpp
                 sqstdsystem.cpp
                 sqstdstring.cpp
                 sqstdaux.cpp
                 sqstdrex.cpp)

add_library(sqstdlib SHARED ${SQSTDLIB_SRC})
target_link_libraries(sqstdlib squirrel)
install(TARGETS sqstdlib RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

add_library(sqstdlib_static STATIC ${SQSTDLIB_SRC})
install(TARGETS sqstdlib_static ARCHIVE DESTINATION lib)
