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
endif()

#Build OHMComm
add_executable( OHMComm ${SRCS} ${HDRS} )

target_link_libraries(OHMComm cpptest)

if(MSVC) 
# Visual Studio Compiler
	target_link_libraries(OHMComm rtaudio_static)
	target_link_libraries(OHMComm WS2_32)
else ()
	# other compilers
	target_link_libraries(OHMComm rtaudio)
endif()


