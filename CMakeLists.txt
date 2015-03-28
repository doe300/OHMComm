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

# Some example applications from rtaudio-devel
if (BUILD_EXAMPLES)
	add_executable (RTAudioProbe examples/audioprobe.cpp)
	target_link_libraries(RTAudioProbe rtaudio)

	add_executable (RTDuplex examples/duplex.cpp)
	target_link_libraries(RTDuplex rtaudio)

	add_executable (Socket examples/socket.cpp)
endif (BUILD_EXAMPLES)
