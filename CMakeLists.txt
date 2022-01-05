#######################
# Main CMakeList.txt  #
#  Regex Performance  #
######################

cmake_minimum_required(VERSION 3.0)

project(RegexPeformance C CXX)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(GENERAL_C_FLAGS "-march=native -Wall -Wstack-usage=5000 -fdiagnostics-color -pipe -fsigned-char -fno-asynchronous-unwind-tables -fno-stack-protector -Wunused-parameter")

set(CMAKE_C_FLAGS "-std=c11 ${GENERAL_C_FLAGS}" CACHE STRING "additional CFLAGS" FORCE)
set(CMAKE_C_FLAGS_DEBUG "-O0 -g")
set(CMAKE_C_FLAGS_RELEASE "-O3")

set(CMAKE_CXX_FLAGS "-std=c++11 ${GENERAL_C_FLAGS}" CACHE STRING "additional CFLAGS" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

## BUILD TYPE
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Release Debug" FORCE)
endif()

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Release" "Debug")

if(NOT CMAKE_BUILD_TYPE MATCHES "^(Release|Debug)$")
    message(FATAL_ERROR "CMAKE_BUILD_TYPE='${CMAKE_BUILD_TYPE}', must be one of Release Debug")
endif()

# Receive the current version from git
execute_process(COMMAND git describe --tags
                OUTPUT_VARIABLE GIT_TAG_VERSION
                OUTPUT_STRIP_TRAILING_WHITESPACE)

string(REGEX REPLACE "([0-9]+)\\.([0-9]+)\\.([0-9]+).*" "\\1" MAJOR_VERSION_GIT ${GIT_TAG_VERSION})
string(REGEX REPLACE "([0-9]+)\\.([0-9]+)\\.([0-9]+).*" "\\2" MINOR_VERSION_GIT ${GIT_TAG_VERSION})
string(REGEX REPLACE "([0-9]+)\\.([0-9]+)\\.([0-9]+).*" "\\3" PATCH_VERSION_GIT ${GIT_TAG_VERSION})

# Write default version numbers. Values can be overwritten on command line.
set(MAJOR_VERSION "${MAJOR_VERSION_GIT}" CACHE STRING "major version")
set(MINOR_VERSION "${MINOR_VERSION_GIT}" CACHE STRING "minor version")
set(PATCH_VERSION "${PATCH_VERSION_GIT}" CACHE STRING "patch version")

string(CONCAT VERSION "${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}")
string(TIMESTAMP BUILD_DATE "%Y-%m-%dT%H:%M:%S")

message("")
message("*******************************************************************")
message("* Regex performance tool version ${VERSION}")
message("* Build type: ${CMAKE_BUILD_TYPE}")
message("* Can be changed with: cmake -DCMAKE_BUILD_TYPE=[Release|Debug]")
message("*******************************************************************")
message("")

add_subdirectory(vendor)
add_subdirectory(src)

message("")

