cmake_minimum_required (VERSION 2.6)

project (OHMComm)






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
endif (BUILD_EXAMPLES)
