# cmake_minimum_required(VERSION <specify CMake version here>)
project(df-fix)
cmake_minimum_required(VERSION 3.16)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 14)

include_directories(".")
include_directories("include")
include_directories("/usr/local/include/eigen3")  #for rocky8
include_directories("../GeographicLib-1.52/include") #for wasm

message("BUILD_TYPE: " ${BUILD_TYPE})
message("COMMIT HASH: " ${COMMIT_HASH})
message("DFFIX VERSION: " ${DFFIX_VERSION})

add_definitions(-DDFFIX_VERSION="--version--")
add_definitions(-DCOMMIT_HASH="--hash--")

include_directories(".")
include_directories("./include")
include_directories("/usr/local/include/eigen3")
include_directories("/usr/include/boost169")
include_directories("/usr/include/python3.6m")

find_library(GeoLib NAMES Geographic)
message("libGeographic: " ${GeoLib})

add_executable(dffix_executable
        fixtest.cpp
        fixlib.cpp
        src/geolib.cpp
        src/DfLocation.cpp
        src/Direction.cpp
        src/DirectionCollection.cpp
        src/DirectionCross.cpp
        src/DirectionCrossCollection.cpp
        src/GeoLocation.cpp
        src/LikelihoodTool.cpp
        src/Logger.cpp
)
target_link_libraries(dffix_executable ${GeoLib})

add_library(dffix SHARED
        fixlib.cpp
        src/geolib.cpp
        src/DfLocation.cpp
        src/Direction.cpp
        src/DirectionCollection.cpp
        src/DirectionCross.cpp
        src/DirectionCrossCollection.cpp
        src/GeoLocation.cpp
        src/LikelihoodTool.cpp
        src/Logger.cpp
)
target_link_libraries(dffix ${GeoLib})



# add_custom_command(TARGET dffix POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:dffix>" ${PROJECT_SOURCE_DIR}/lib/libdffix_${BUILD_TYPE}.so)


# # if (${BUILD_TYPE} STREQUAL "wasm")

# #         add_library(dffix STATIC
# #                 fixlib.cpp
# #                 src/geolib.cpp
# #                 src/DfLocation.cpp
# #                 src/Direction.cpp
# #                 src/DirectionCollection.cpp
# #                 src/DirectionCross.cpp
# #                 src/DirectionCrossCollection.cpp
# #                 src/GeoLocation.cpp
# #                 src/LikelihoodTool.cpp
# #                 src/Logger.cpp
# #                 )
# #         target_link_libraries(dffix "../GeographicLib-1.52/src/.libs/libGeographic.a")
# #         add_custom_command(TARGET dffix POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:dffix>" ${PROJECT_SOURCE_DIR}/lib/libdffix_${BUILD_TYPE}.a)
# # else()
# #         find_library(GeoLib NAMES Geographic)
# #         message("libGeographic: " ${GeoLib})

# #         add_library(dffix SHARED
# #                 fixlib.cpp
# #                 src/geolib.cpp
# #                 src/DfLocation.cpp
# #                 src/Direction.cpp
# #                 src/DirectionCollection.cpp
# #                 src/DirectionCross.cpp
# #                 src/DirectionCrossCollection.cpp
# #                 src/GeoLocation.cpp
# #                 src/LikelihoodTool.cpp
# #                 src/Logger.cpp
# #                 )
# #         target_link_libraries(dffix ${GeoLib})
# #         add_custom_command(TARGET dffix POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:dffix>" ${PROJECT_SOURCE_DIR}/lib/libdffix_${BUILD_TYPE}.so)
# # endif()
