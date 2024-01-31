#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "squirrel::squirrel" for configuration "Release"
set_property(TARGET squirrel::squirrel APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(squirrel::squirrel PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/squirrel.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/squirrel.dll"
  )

list(APPEND _cmake_import_check_targets squirrel::squirrel )
list(APPEND _cmake_import_check_files_for_squirrel::squirrel "${_IMPORT_PREFIX}/lib/squirrel.lib" "${_IMPORT_PREFIX}/bin/squirrel.dll" )

# Import target "squirrel::squirrel_static" for configuration "Release"
set_property(TARGET squirrel::squirrel_static APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(squirrel::squirrel_static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/squirrel_static.lib"
  )

list(APPEND _cmake_import_check_targets squirrel::squirrel_static )
list(APPEND _cmake_import_check_files_for_squirrel::squirrel_static "${_IMPORT_PREFIX}/lib/squirrel_static.lib" )

# Import target "squirrel::sqstdlib" for configuration "Release"
set_property(TARGET squirrel::sqstdlib APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(squirrel::sqstdlib PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/sqstdlib.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/sqstdlib.dll"
  )

list(APPEND _cmake_import_check_targets squirrel::sqstdlib )
list(APPEND _cmake_import_check_files_for_squirrel::sqstdlib "${_IMPORT_PREFIX}/lib/sqstdlib.lib" "${_IMPORT_PREFIX}/bin/sqstdlib.dll" )

# Import target "squirrel::sqstdlib_static" for configuration "Release"
set_property(TARGET squirrel::sqstdlib_static APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(squirrel::sqstdlib_static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/sqstdlib_static.lib"
  )

list(APPEND _cmake_import_check_targets squirrel::sqstdlib_static )
list(APPEND _cmake_import_check_files_for_squirrel::sqstdlib_static "${_IMPORT_PREFIX}/lib/sqstdlib_static.lib" )

# Import target "squirrel::interpreter" for configuration "Release"
set_property(TARGET squirrel::interpreter APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(squirrel::interpreter PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/sq.exe"
  )

list(APPEND _cmake_import_check_targets squirrel::interpreter )
list(APPEND _cmake_import_check_files_for_squirrel::interpreter "${_IMPORT_PREFIX}/bin/sq.exe" )

# Import target "squirrel::interpreter_static" for configuration "Release"
set_property(TARGET squirrel::interpreter_static APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(squirrel::interpreter_static PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/sq_static.exe"
  )

list(APPEND _cmake_import_check_targets squirrel::interpreter_static )
list(APPEND _cmake_import_check_files_for_squirrel::interpreter_static "${_IMPORT_PREFIX}/bin/sq_static.exe" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
