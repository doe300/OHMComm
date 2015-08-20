include_directories(..)
include_directories(../include)

#Include the source files
file( GLOB SRCS *.cpp *.h)

#Visual Studio specific settings
if(MSVC)
	link_directories ("${PROJECT_BINARY_DIR}/build/rtaudio/Debug")
	link_directories ("${PROJECT_BINARY_DIR}/build/rtaudio/Release")
	link_directories ("${PROJECT_BINARY_DIR}/build/cpptest/Debug")
	link_directories ("${PROJECT_BINARY_DIR}/build/cpptest/Release")
	link_directories ("${PROJECT_BINARY_DIR}/build/opus/Debug")
	link_directories ("${PROJECT_BINARY_DIR}/build/opus/Release")
endif()

add_executable(Tests ${SRCS})
target_link_libraries(Tests cpptest)
target_link_libraries(Tests OHMCommLib)
target_link_libraries(Tests opus)
target_link_libraries(Tests celt)
target_link_libraries(Tests silk_common)
target_link_libraries(Tests silk_fixed)
target_link_libraries(Tests silk_float)
