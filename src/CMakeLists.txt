#Include headers in the project structure
file( GLOB HDRS "${PROJECT_SOURCE_DIR}/include/*.h" )

#Include headers in the project settings (as search-path for header-files)
include_directories ("${PROJECT_SOURCE_DIR}/include")

#Include the source files
file( GLOB SRCS *.cpp )

#Visual Studio specific settings
if(MSVC)
	link_directories ("${PROJECT_BINARY_DIR}/build/rtaudio/Debug")
	link_directories ("${PROJECT_BINARY_DIR}/build/rtaudio/Release")
	link_directories ("${PROJECT_BINARY_DIR}/build/cpptest/Debug")
	link_directories ("${PROJECT_BINARY_DIR}/build/cpptest/Release")
	link_directories ("${PROJECT_BINARY_DIR}/build/opus/Debug")
	link_directories ("${PROJECT_BINARY_DIR}/build/opus/Release")
endif()

#Build OHMComm
add_executable( OHMComm ${SRCS} ${HDRS} )
#Build OHMCommLib for usage with Tests
add_library( OHMCommLib STATIC ${SRCS} ${HDRS} )

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
target_link_libraries(OHMComm celt)
target_link_libraries(OHMComm silk_common)
target_link_libraries(OHMComm silk_fixed)
target_link_libraries(OHMComm silk_float)
target_link_libraries(OHMCommLib opus)
target_link_libraries(OHMCommLib celt)
target_link_libraries(OHMCommLib silk_common)
target_link_libraries(OHMCommLib silk_fixed)
target_link_libraries(OHMCommLib silk_float)
