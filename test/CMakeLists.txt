include_directories(..)
include_directories(../include)

#Include the source files
file( GLOB SRCS *.cpp *.h)

option(TEST_OUTPUT_CONSOLE "Prints the test-output to console" OFF)
if(TEST_OUTPUT_CONSOLE)
	#Create macro-definition to enable output to console
	add_definitions(-DTEST_OUTPUT_CONSOLE=1)
endif()

#Visual Studio specific settings
if(MSVC)
	link_directories ("${PROJECT_BINARY_DIR}/build/rtaudio/Debug")
	link_directories ("${PROJECT_BINARY_DIR}/build/rtaudio/Release")
	#link_directories ("${PROJECT_BINARY_DIR}/build/cpptest/Debug")
	#link_directories ("${PROJECT_BINARY_DIR}/build/cpptest/Release")
	link_directories ("${PROJECT_BINARY_DIR}/build/cpptest-lite/Debug")
	link_directories ("${PROJECT_BINARY_DIR}/build/cpptest-lite/Release")
	link_directories ("${PROJECT_BINARY_DIR}/build/opus/Debug")
	link_directories ("${PROJECT_BINARY_DIR}/build/opus/Release")
endif()

#include_directories ("${PROJECT_SOURCE_DIR}/lib/cpptest-1.1.2/src")
include_directories ("${PROJECT_SOURCE_DIR}/lib/cpptest-lite/include")

add_executable(Tests ${SRCS})
#target_link_libraries(Tests cpptest)
target_link_libraries(Tests cpptest-lite)
target_link_libraries(Tests OHMCommLib)
target_link_libraries(Tests opus)
target_link_libraries(Tests celt)
target_link_libraries(Tests silk_common)
target_link_libraries(Tests silk_fixed)
target_link_libraries(Tests silk_float)
