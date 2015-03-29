
#Include all headers from the ./include/ directory
include_directories ("${PROJECT_SOURCE_DIR}/include")

#RTP protocol
add_library(RTP RTPWrapper.cpp)

#AudioProcessors
add_library(AudioProcessors AudioProcessor.cpp)

####
# Main Program
####
add_executable(OHMComm OHMComm.cpp)
target_link_libraries(OHMComm rtaudio)
target_link_libraries(OHMComm RTP)
target_link_libraries(OHMComm AudioProcessors)