add_executable(sq sq.c)
set_target_properties(sq PROPERTIES LINKER_LANGUAGE C)
target_link_libraries(sq squirrel sqstd)
install(TARGETS sq RUNTIME DESTINATION bin)

add_executable(sq_static sq.c)
set_target_properties(sq_static PROPERTIES LINKER_LANGUAGE C)
target_link_libraries(sq_static squirrel_static sqstd_static)

if(CMAKE_COMPILER_IS_GNUCXX)
  set_target_properties(sq_static PROPERTIES COMPILE_FLAGS "-static -Wl,-static")
endif()

install(TARGETS sq_static RUNTIME DESTINATION bin)
