
#Include all headers from the ./include/ directory
include_directories ("${PROJECT_SOURCE_DIR}/include")

####
# AudioProcessors
####
add_library(AudioProcessors AudioProcessor.cpp)

####
# NetworkWrappers
####
add_library(Network NetworkWrapper.cpp)
#RTP protocol
add_library(RTP RTPWrapper.cpp)
#UDP Protocol
add_library(UDP UDPWrapper.cpp)
target_link_libraries(UDP Network)

####
# Main Program
####
add_executable(OHMComm OHMComm.cpp)
target_link_libraries(OHMComm rtaudio)
target_link_libraries(OHMComm RTP)
target_link_libraries(OHMComm UDP)
target_link_libraries(OHMComm AudioProcessors)