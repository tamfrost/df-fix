project(df-fix-wasm)
cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_VERBOSE_MAKEFILE ON)

include_directories("../..")
include_directories("../../include")
include_directories("/usr/local/include")
include_directories("/usr/include/eigen3")

if(NOT DEFINED GEOFIX_VERSION)
  set(GEOFIX_VERSION "dev")
endif()
if(NOT DEFINED COMMIT_HASH)
  set(COMMIT_HASH "unknown")
endif()

add_definitions(
  -DGEOFIX_VERSION="${GEOFIX_VERSION}"
  -DCOMMIT_HASH="${COMMIT_HASH}"
)

find_library(GeoLib NAMES GeographicLib HINTS /usr/local/lib)
if(NOT GeoLib)
  message(FATAL_ERROR "GeographicLib not found")
endif()

add_executable(fix
    fixbind.cpp
    ../../src/fixlib.cpp
    ../../src/geolib.cpp
    ../../src/Bearing.cpp
    ../../src/DfLocation.cpp
    ../../src/DfSite.cpp
    ../../src/Direction.cpp
    ../../src/DirectionCollection.cpp
    ../../src/DirectionCross.cpp
    ../../src/DirectionCrossCollection.cpp
    ../../src/GeoLocation.cpp
    ../../src/LikelihoodTool.cpp
    ../../src/Logger.cpp
)

target_link_libraries(fix ${GeoLib})

set_target_properties(fix PROPERTIES LINK_FLAGS "--bind -O3 \
  -s WASM=1 \
  -s DISABLE_EXCEPTION_CATCHING=0 \
  -s EXCEPTION_CATCHING_ALLOWED='[\"*\"]' \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s EXPORTED_FUNCTIONS=\"['_malloc','_free']\" \
  -s EXPORTED_RUNTIME_METHODS=\"['ccall', 'cwrap', 'intArrayFromString', 'allocate', 'ALLOC_NORMAL', 'UTF8ToString', 'writeArrayToMemory', 'stringToUTF8']\" \
  -s MODULARIZE=1 \
  -s EXPORT_NAME=fixModule \
  -s ASSERTIONS=1 \
  -s SINGLE_FILE=1")
