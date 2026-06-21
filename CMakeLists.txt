cmake_minimum_required(VERSION 3.16)
project(df-fix-api)
set(CMAKE_CXX_STANDARD 17)

if(NOT DEFINED DFFIX_VERSION)
  set(DFFIX_VERSION "dev")
endif()
if(NOT DEFINED COMMIT_HASH)
  set(COMMIT_HASH "unknown")
endif()

add_definitions(
  -DDFFIX_VERSION="${DFFIX_VERSION}"
  -DGEOFIX_VERSION="${DFFIX_VERSION}"
  -DCOMMIT_HASH="${COMMIT_HASH}"
  -DASIO_STANDALONE
)

find_package(Threads REQUIRED)
find_library(GeoLib NAMES GeographicLib HINTS /usr/local/lib)
if(NOT GeoLib)
  message(FATAL_ERROR "GeographicLib (libGeographicLib) not found in /usr/local/lib")
endif()

include_directories(include /usr/local/include /usr/include/eigen3)

set(LIB_SOURCES
    src/fixlib.cpp
    src/geolib.cpp
    src/Bearing.cpp
    src/DfLocation.cpp
    src/DfSite.cpp
    src/Direction.cpp
    src/DirectionCollection.cpp
    src/DirectionCross.cpp
    src/DirectionCrossCollection.cpp
    src/GeoLocation.cpp
    src/LikelihoodTool.cpp
    src/Logger.cpp
)

add_executable(df-fix-api src/main.cpp ${LIB_SOURCES})
target_link_libraries(df-fix-api PRIVATE Threads::Threads ${GeoLib})
