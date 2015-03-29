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
# Main files
####
#build all from ./src into ./build
add_subdirectory(src build)


####
# Example and Testing applications
####
set (BUILD_EXAMPLES 1)

# Some example applications
if (BUILD_EXAMPLES)
	#build all ./examples into ./build/examples
	add_subdirectory(examples build/examples)
endif (BUILD_EXAMPLES)
