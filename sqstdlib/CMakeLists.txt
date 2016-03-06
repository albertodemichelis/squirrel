set(SQSTD_SRC sqstdblob.cpp
              sqstdio.cpp
              sqstdstream.cpp
              sqstdmath.cpp
              sqstdsystem.cpp
              sqstdstring.cpp
              sqstdaux.cpp
              sqstdrex.cpp)

add_library(sqstd SHARED ${SQSTD_SRC})
target_link_libraries(sqstd squirrel)
install(TARGETS sqstd LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

add_library(sqstd_static STATIC ${SQSTD_SRC})
install(TARGETS sqstd_static ARCHIVE DESTINATION lib)
