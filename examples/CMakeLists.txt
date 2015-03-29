add_executable (RTAudioProbe audioprobe.cpp)
target_link_libraries(RTAudioProbe rtaudio)

add_executable (RTDuplex duplex.cpp)
target_link_libraries(RTDuplex rtaudio)

add_executable (Socket socket.cpp)