cmake_minimum_required (VERSION 2.6)

project (OHMComm)

####
# General configuration
####
# build as shared libraries
SET(BUILD_SHARED_LIBS ON)
#append usage of C++ to compiler flags, also optimize for speed and enable all warnings
LIST( APPEND CMAKE_CXX_FLAGS "-std=c++0x -O3 -Wall ${CMAKE_CXX_FLAGS}")

####
# Dependencies
####
add_subdirectory(lib/rtaudio-4.1.1 build/rtaudio)


####
# Main files
####
#build all from ./src into ./build
add_subdirectory(src build)