cmake_minimum_required (VERSION 2.6)

project (OHMComm)

####
# General configuration
####
# build as shared libraries
SET(BUILD_SHARED_LIBS ON)
# Option to enable/disable test-program
option(BUILD_TESTING "Build testing program" ON)

# append usage of C++ to compiler flags, also optimize for speed and enable all warnings
if(NOT MSVC)
	# the following instruction causes warnings in visual studio, 
	# therefore enable it only when MSVC-compiler is not used
	LIST(APPEND CMAKE_CXX_FLAGS "-std=c++0x -O3 -Wall -pthread")
else()
	# visual studio specific flags
	set(CMAKE_CXX_FLAGS "/W3 /D_UNICODE /DUNICODE")
	add_definitions(-DWIN32_LEAN_AND_MEAN)
endif()

####
# Dependencies
####
# Set output copy path
add_subdirectory(lib/rtaudio-4.1.1 build/rtaudio)
add_subdirectory(lib/opus-1.1 build/opus)

#Include headers in the project settings (as search-path for header-files)
include_directories ("${PROJECT_SOURCE_DIR}/lib/rtaudio-4.1.1")
include_directories ("${PROJECT_SOURCE_DIR}/lib/opus-1.1/include")

####
# Main files
####
#build all from ./src into ./build
add_subdirectory(src build)

if (BUILD_TESTING)
	add_subdirectory(lib/cpptest-lite build/cpptest-lite)
    add_subdirectory(test build/test)
endif (BUILD_TESTING)