cmake_minimum_required(VERSION 2.8)
project(libsquirrel)

# sources
set(SQ_SRCS
    sq.c
)

# libs
set(SQ_LIBS
    sqstd
    squirrel
)

# shared lib
add_executable(sq ${SQ_SRCS})
target_link_libraries(sq ${SQ_LIBS})

# static lib
add_executable(sq_static ${SQ_SRCS})
set_target_properties(sq_static PROPERTIES COMPILE_FLAGS "-static -Wl,-static")
target_link_libraries(sq_static ${SQ_LIBS})
