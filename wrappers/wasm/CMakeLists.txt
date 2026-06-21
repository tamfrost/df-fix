# cmake_minimum_required(VERSION <specify CMake version here>)
project(geofix)
cmake_minimum_required(VERSION 3.16)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 14)

include_directories("../..")
include_directories("../../include")
include_directories("/usr/local/include")
include_directories("/usr/include/eigen3")
#include_directories("/emsdk/upstream/emscripten/system/include")

message("BUILD_TYPE: " ${BUILD_TYPE})
message("COMMIT HASH: " ${COMMIT_HASH})
message("GEOFIX VERSION: " ${GEOFIX_VERSION})

add_definitions(-DGEOFIX_VERSION="${GEOFIX_VERSION}")
add_definitions(-DCOMMIT_HASH="${COMMIT_HASH}")

# Not working for some reason
#find_library(geographicLib NAME libGeographic.a PATH /usr/lib/x86_64-linux-gnu)
#message(${geographicLib})

# add_executable(geofix geofixbind.cpp)
# target_link_libraries(geofix "/usr/local/lib/libgeofix_wasm.a")
add_executable(geofix
        geofixbind.cpp
        ../../fixlib.cpp
        ../../src/geolib.cpp
        ../../src/DfLocation.cpp
        ../../src/Direction.cpp
        ../../src/DirectionCollection.cpp
        ../../src/DirectionCross.cpp
        ../../src/DirectionCrossCollection.cpp
        ../../src/GeoLocation.cpp
        ../../src/LikelihoodTool.cpp
        ../../src/Logger.cpp
        ../../src/Bearing.cpp
)

target_link_libraries(geofix "/usr/lib/x86_64-linux-gnu/libGeographic.a")

set_target_properties(geofix PROPERTIES LINK_FLAGS "--bind -O3 \
-s WASM=1 \
-s DISABLE_EXCEPTION_CATCHING=0 \
-s EXCEPTION_CATCHING_ALLOWED='[\"*\"]' \
-s ALLOW_MEMORY_GROWTH=1 \
-s EXPORTED_FUNCTIONS=\"['_malloc']\" \
-s EXPORTED_RUNTIME_METHODS=\"['ccall', 'cwrap', 'intArrayFromString', 'allocate', 'ALLOC_NORMAL', 'UTF8ToString', 'writeArrayToMemory', 'stringToUTF8']\" \
-s MODULARIZE=1 \
-s EXPORT_NAME=geoModule \
-s ASSERTIONS=1 \
-s SINGLE_FILE=1")
add_custom_command(TARGET geofix POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:geofix>" ${PROJECT_SOURCE_DIR}/package/lib/libgeofix_${BUILD_TYPE}.js)
