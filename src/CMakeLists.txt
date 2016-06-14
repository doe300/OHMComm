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
	#MSVC seems to be unable to link to shared library out of the box
	add_library( OHMComm STATIC ${SRCS} ${HDRS} )
else()
	#Build shared library OHMComm
	add_library( OHMComm SHARED ${SRCS} ${HDRS} )
endif()

#Build OHMComm
add_executable( OHMCommStandalone "OHMCommStandalone.cpp" )
target_link_libraries(OHMCommStandalone OHMComm)

if(MSVC) 
# Visual Studio Compiler
	target_link_libraries(OHMCommStandalone rtaudio_static)
	target_link_libraries(OHMCommStandalone WS2_32)
	target_link_libraries(OHMComm rtaudio_static)
	target_link_libraries(OHMComm WS2_32)
else ()
	# other compilers
	target_link_libraries(OHMComm rtaudio)
endif()

#Common libraries
target_link_libraries(OHMComm opus)
if(NOT OPUS_LIBRARY) #Only manually link opus, if it not provided via a library
	target_link_libraries(OHMComm celt)
	target_link_libraries(OHMComm silk_common)
	target_link_libraries(OHMComm silk_fixed)
	target_link_libraries(OHMComm silk_float)
endif()

# Only link libraries, if available
if(PORTAUDIO_LIBRARY)
	target_link_libraries(OHMComm portaudio)
endif()
if(ILBC_LIBRARY)
	target_link_libraries(OHMComm ilbc)
endif()
if(GSM_LIBRARY)
	target_link_libraries(OHMComm gsm)
endif()
if(AMR_LIBRARY)
	target_link_libraries(OHMComm opencore-amrnb)
endif()
