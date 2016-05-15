include_directories(..)
include_directories(../include)

#Include the source files
file( GLOB SRCS *.cpp */*.cpp *.h */*.h)

option(TEST_OUTPUT_CONSOLE "Prints the test-output to console" OFF)
option(EXTENDED_SIP_TEST "Extended Test for the SIP protocol" OFF)
if(TEST_OUTPUT_CONSOLE)
	#Create macro-definition to enable output to console
	add_definitions(-DTEST_OUTPUT_CONSOLE=1)
endif()
if(EXTENDED_SIP_TEST)
	#Create macro-definition to enable extended SIP testing
	add_definitions(-DEXTENDED_SIP_TEST=1)
endif()

#Visual Studio specific settings
if(MSVC)
	link_directories ("${PROJECT_BINARY_DIR}/build/rtaudio/Debug")
	link_directories ("${PROJECT_BINARY_DIR}/build/rtaudio/Release")
	link_directories ("${PROJECT_BINARY_DIR}/build/cpptest-lite/Debug")
	link_directories ("${PROJECT_BINARY_DIR}/build/cpptest-lite/Release")
	link_directories ("${PROJECT_BINARY_DIR}/build/opus/Debug")
	link_directories ("${PROJECT_BINARY_DIR}/build/opus/Release")
endif()

include_directories ("${PROJECT_SOURCE_DIR}/lib/cpptest-lite/include")

add_executable(Tests ${SRCS})
target_link_libraries(Tests cpptest-lite)
target_link_libraries(Tests OHMComm)
target_link_libraries(Tests opus)
if(NOT OPUS_LIBRARY) #Only manually link opus, if it not provided via a library
	target_link_libraries(Tests celt)
	target_link_libraries(Tests silk_common)
	target_link_libraries(Tests silk_fixed)
	target_link_libraries(Tests silk_float)
endif()
