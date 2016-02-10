#Include headers in the project structure
file( GLOB HDRS "${PROJECT_SOURCE_DIR}/include/*.h" )

#Include headers in the project settings (as search-path for header-files)
include_directories ("${PROJECT_SOURCE_DIR}/include")

#Include the source files
file( GLOB SRCS *.cpp */*.cpp )

#Visual Studio specific settings
if(MSVC)
	link_directories ("${PROJECT_BINARY_DIR}/build/rtaudio/Debug")
	link_directories ("${PROJECT_BINARY_DIR}/build/rtaudio/Release")
	link_directories ("${PROJECT_BINARY_DIR}/build/cpptest/Debug")
	link_directories ("${PROJECT_BINARY_DIR}/build/cpptest/Release")
	link_directories ("${PROJECT_BINARY_DIR}/build/opus/Debug")
	link_directories ("${PROJECT_BINARY_DIR}/build/opus/Release")
endif()

#Build OHMCommLib
add_library( OHMCommLib STATIC ${SRCS} ${HDRS} )

#Build OHMComm
add_executable( OHMComm "OHMCommStandalone.cpp" )
target_link_libraries(OHMComm OHMCommLib)

if(MSVC) 
# Visual Studio Compiler
	target_link_libraries(OHMComm rtaudio_static)
	target_link_libraries(OHMComm WS2_32)
	target_link_libraries(OHMCommLib rtaudio_static)
	target_link_libraries(OHMCommLib WS2_32)
else ()
	# other compilers
	target_link_libraries(OHMComm rtaudio)
	target_link_libraries(OHMCommLib rtaudio)
endif()

#Common libraries
target_link_libraries(OHMComm opus)
if(NOT OPUS_LIBRARY) #Only manually link opus, if it not provided via a library
	target_link_libraries(OHMComm celt)
	target_link_libraries(OHMComm silk_common)
	target_link_libraries(OHMComm silk_fixed)
	target_link_libraries(OHMComm silk_float)
endif()
target_link_libraries(OHMCommLib opus)
if(NOT OPUS_LIBRARY) #Only manually link opus, if it not provided via a library
	target_link_libraries(OHMCommLib celt)
	target_link_libraries(OHMCommLib silk_common)
	target_link_libraries(OHMCommLib silk_fixed)
	target_link_libraries(OHMCommLib silk_float)
endif()
if(PORTAUDIO_LIBRARY)
	target_link_libraries(OHMCommLib portaudio)
endif()