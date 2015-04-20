cmake_minimum_required (VERSION 2.6)

project (OHMComm)

####
# General configuration
####
# build as shared libraries
SET(BUILD_SHARED_LIBS ON)

# append usage of C++ to compiler flags, also optimize for speed and enable all warnings
if(NOT MSVC)
	# the following instruction causes warnings in visual studio, 
	# therefore enable it only when MSVC-compiler is not used
	LIST(APPEND CMAKE_CXX_FLAGS "-std=c++0x -O3 -Wall")
else()
	# visual studio specific flags
	set(CMAKE_CXX_FLAGS "/EHsc /W3")
endif()

####
# Dependencies
####
add_subdirectory(lib/rtaudio-4.1.1 build/rtaudio)

#Include headers in the project settings (as search-path for header-files)
include_directories ("${PROJECT_SOURCE_DIR}/lib/rtaudio-4.1.1")

####
# Main files
####
#build all from ./src into ./build
add_subdirectory(src build)