cmake_minimum_required (VERSION 2.6)

project (OHMComm)






####
# Example and Testing applications
####

option (BUILD_EXAMPLES "Build examples and testing applications" ON)

# Some example applications from rtaudio-devel
if (BUILD_EXAMPLES)
	add_executable (RTAudioProbe test/audioprobe.cpp)
	target_link_libraries(RTAudioProbe rtaudio)

	add_executable (RTDuplex test/duplex.cpp)
	target_link_libraries(RTDuplex rtaudio)
endif (BUILD_EXAMPLES)
