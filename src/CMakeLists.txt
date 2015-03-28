
#Include all headers from the ./include/ directory
include_directories ("${PROJECT_SOURCE_DIR}/include")

#RTP protocol
add_library(RTPWrapper RTPWrapper.cpp)