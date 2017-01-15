cmake_minimum_required (VERSION 2.6)
project (xritDecoder)
enable_language(CXX)

# C++11 Macro
macro(use_cxx11)
  if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      set (CMAKE_CXX_FLAGS "--std=gnu++11 ${CMAKE_CXX_FLAGS}")
    endif ()
  else ()
    set (CMAKE_CXX_STANDARD 11)
  endif ()
endmacro(use_cxx11)

use_cxx11()

# Version
set(VERSION_INFO_MAJOR_VERSION 1)
set(VERSION_INFO_MINOR_VERSION 3)
set(VERSION_INFO_MAINT_VERSION 0git)

# Flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

# Source Code
file(GLOB SOURCES src/*.cpp)

add_executable (${PROJECT_NAME} ${SOURCES})

# Links
target_link_libraries (${PROJECT_NAME} SatHelper)
target_link_libraries (${PROJECT_NAME} pthread)

# Install library
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# Debian Package
SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Lucas Teske")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libcorrect")
SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION "SatHelper library for Satellite Projects")
INCLUDE(CPack)