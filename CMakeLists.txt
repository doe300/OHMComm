cmake_minimum_required (VERSION 3.2)

project (OHMComm)

####
# General configuration
####
# build as shared libraries
SET(BUILD_SHARED_LIBS ON)
# Option to enable/disable test-program
option(BUILD_TESTING "Build testing program" ON)
option(BUILD_DEBUG "Build with debugging symbols. Otherwise build for performance" OFF)
option(FORCE_CUSTOM_LIBRARIES "Force the use of custom libraries instead of the system-provided. Use this if your system ships with outdated versions of the libraries" OFF)

# append usage of C++ to compiler flags, also optimize for speed and enable all warnings
if(NOT MSVC)
	# the following instruction causes warnings in visual studio, 
	# therefore enable it only when MSVC-compiler is not used
	if(BUILD_DEBUG)
		LIST(APPEND CMAKE_CXX_FLAGS "-std=c++0x -g3 -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -pthread -fPIC -Wpedantic")
		LIST(APPEND CMAKE_C_FLAGS "-g3 -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -pthread -fPIC -Wpedantic")
	else()
		LIST(APPEND CMAKE_CXX_FLAGS "-std=c++0x -O3 -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -pthread -fPIC -Wpedantic")
		LIST(APPEND CMAKE_C_FLAGS "-O3 -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -pthread -fPIC -Wpedantic")
	endif()
else()
	# visual studio specific flags
	set(CMAKE_CXX_FLAGS "/W3 /D_UNICODE /DUNICODE /DNOMINMAX /EHsc")
	add_definitions(-DWIN32_LEAN_AND_MEAN)
endif()

####
# Dependencies
####
set(AUDIO_LIBRARY_FOUND OFF)
set(CRYPTO_LIBRARY_FOUND OFF)
set(SUBMODULES_UPDATED OFF)

#
# audio libraries
#

if(NOT FORCE_CUSTOM_LIBRARIES)
	# RTAudio - http://www.music.mcgill.ca/~gary/rtaudio/
	#Try to find precompiled library
	find_library(RTAUDIO_LIBRARY NAMES rtaudio librtaudio)
	if(RTAUDIO_LIBRARY)
		message(STATUS "RTAudio library found: " ${RTAUDIO_LIBRARY})
		#Set header to default include path
		add_definitions(-DRTAUDIO_HEADER="RtAudio.h")
		set(AUDIO_LIBRARY_FOUND ON)
	endif()

	# PortAudio - http://www.portaudio.com/
	#Try to find precompiled library
	find_library(PORTAUDIO_LIBRARY NAMES portaudio libportaudio)
	if(PORTAUDIO_LIBRARY)
		message(STATUS "PortAudio library found: " ${PORTAUDIO_LIBRARY})
		#Set header to default include path
		add_definitions(-DPORTAUDIO_HEADER="portaudio.h")
		set(AUDIO_LIBRARY_FOUND ON)
	endif()
endif()

#Force custom library or library not found
if(NOT AUDIO_LIBRARY_FOUND)
	#Update custom library
	if(NOT SUBMODULES_UPDATED)
		include(cmake/submodules.cmake)
		set(SUBMODULES_UPDATED ON)
	endif()

	# RTAudio - http://www.music.mcgill.ca/~gary/rtaudio/
	#use manually downloaded library
	find_path(RTAUDIO_HEADER_PATH "RtAudio.h" HINTS lib/rtaudio)
	if(RTAUDIO_HEADER_PATH)
		message(STATUS "Compiling provided rtaudio-library!")
		# Compile ourselves and set correct header-path
		add_subdirectory(lib/rtaudio build/rtaudio)
		add_definitions(-DRTAUDIO_HEADER="../lib/rtaudio/RtAudio.h")
		set(AUDIO_LIBRARY_FOUND ON)
	endif()
endif()

# Check if any audio-library was added
if(NOT AUDIO_LIBRARY_FOUND)
	message(SEND_ERROR "No supported audio-library found!")
endif()

#
# audio codecs
#

# Opus - http://opus-codec.org/
if(NOT FORCE_CUSTOM_LIBRARIES)
	#Try to find precompiled library
	find_library(OPUS_LIBRARY NAMES opus libopus)
	if(OPUS_LIBRARY)
		message(STATUS "Opus library found: " ${OPUS_LIBRARY})
		#Set header to default include path
		add_definitions(-DOPUS_HEADER="opus/opus.h")
	endif()
endif()
#Force custom library or library not found
if(FORCE_CUSTOM_LIBRARIES OR NOT OPUS_LIBRARY)
	#Update custom library
	if(NOT SUBMODULES_UPDATED)
		include(cmake/submodules.cmake)
		set(SUBMODULES_UPDATED ON)
	endif()

	#Opus doesn't provide a CMakeLists.txt (anymore??), so copy it manually
	execute_process(
        COMMAND             cmake -E copy cmake/CMakeLists-opus.txt lib/opus/CMakeLists.txt
        WORKING_DIRECTORY   ${PROJECT_SOURCE_DIR}
    )
	#version.h for Windows is usually generated automatically -> doesn't work (at least on Appveyor)
	execute_process(
        COMMAND             cmake -E copy cmake/opus-version.h lib/opus/win32/version.h
        WORKING_DIRECTORY   ${PROJECT_SOURCE_DIR}
    )

	find_path(OPUS_HEADER_PATH "opus.h" HINTS lib/opus/include)
	if(OPUS_HEADER_PATH)
		message(STATUS "Compiling provided opus-library!")
		# Compile ourselves and set correct header-path
		add_subdirectory(lib/opus build/opus)
		add_definitions(-DOPUS_HEADER="../lib/opus/include/opus.h")
	endif()
endif()
if(NOT OPUS_LIBRARY AND NOT OPUS_HEADER_PATH)
	message(WARNING "No opus library found")
endif()

# iLBC - https://tools.ietf.org/html/rfc3951
#Try to find precompiled library
find_library(ILBC_LIBRARY NAMES ilbc libilbc)
if(ILBC_LIBRARY)
	message(STATUS "iLBC library found: " ${ILBC_LIBRARY})
	#Set header to default include path
	add_definitions(-DILBC_HEADER="ilbc.h")
endif()

# AMR - https://sourceforge.net/projects/opencore-amr/
#Try to find precompiled library
find_library(AMR_LIBRARY NAMES amr "opencore-amrnb" "libopencore-amrnb")
if(AMR_LIBRARY)
	message(STATUS "opencore-AMR library found: " ${AMR_LIBRARY})
	add_definitions(-DAMR_ENCODER_HEADER="opencore-amrnb/interf_enc.h")
	add_definitions(-DAMR_DECODER_HEADER="opencore-amrnb/interf_dec.h")
endif()

# GSM - http://www.quut.com/gsm/
#Try to find precompiled library
find_library(GSM_LIBRARY NAMES gsm libgsm)
if(GSM_LIBRARY)
	message(STATUS "GSM library found: " ${GSM_LIBRARY})
	#Set header to default include path
	add_definitions(-DGSM_HEADER="gsm/gsm.h")
endif()

#
# other
#

if(NOT FORCE_CUSTOM_LIBRARIES)
	# OpenSSL - https://www.openssl.org/
	#Use CMake predefined finder-module
	include(FindOpenSSL)
	if(OPENSSL_FOUND AND OPENSSL_INCLUDE_DIR)
		message(STATUS "OpenSSL crypto library found: " ${OPENSSL_CRYPTO_LIBRARY} " in version " ${OPENSSL_VERSION})
		#Set header to default include path
		add_definitions(-DOPENSSL_CRYPTO_LIBRARY="1")
		include_directories(${OPENSSL_INCLUDE_DIR})
		set(CRYPTO_LIBRARY_FOUND ON)
	endif()

	# Cryptopp - https://www.cryptopp.com/
	if(NOT CRYPTO_LIBRARY_FOUND)
		#Try to find precompiled library
		find_library(CRYPTOPP_LIBRARY NAMES cryptopp libcryptopp)
		if(CRYPTOPP_LIBRARY)
			message(STATUS "Crypto++ library found: " ${CRYPTOPP_LIBRARY})
			#Set header to default include path
			add_definitions(-DCRYPTOPP_LIBRARY="1")
			set(CRYPTO_LIBRARY_FOUND ON)
		endif()
	endif()
endif()

#Force custom library or library not found
if(NOT CRYPTO_LIBRARY_FOUND)
	#Update custom library
	if(NOT SUBMODULES_UPDATED)
		include(cmake/submodules.cmake)
		set(SUBMODULES_UPDATED ON)
	endif()

	#Cryptopp doesn't provide a CMakeLists.txt (at least not yet for version 5.6.3), so copy it manually
	execute_process(
        COMMAND             cmake -E copy cmake/CMakeLists-cryptopp.txt lib/cryptopp/CMakeLists.txt
        WORKING_DIRECTORY   ${PROJECT_SOURCE_DIR}
    )

	# Cryptopp - https://www.cryptopp.com/
	#use manually downloaded library
	find_path(CRYPTOPP_HEADER_PATH "aes.h" HINTS lib/cryptopp)
	if(CRYPTOPP_HEADER_PATH)
		message(STATUS "Compiling provided Crypto++-library!")
		# Compile ourselves and set correct header-path
		add_subdirectory(lib/cryptopp build/cryptopp)
		add_definitions(-DCRYPTOPP_LIBRARY="1")
		set(CRYPTO_LIBRARY_FOUND ON)
	endif()
endif()

# Check if any crypto-library was added
if(NOT CRYPTO_LIBRARY_FOUND)
	message(WARNING "No supported crypto-library found!")
endif()

####
# Main files
####
#build all from ./src into ./build
add_subdirectory(src build)

if (BUILD_TESTING)
	if(NOT SUBMODULES_UPDATED)
		include(cmake/submodules.cmake)
	endif()
	add_subdirectory(lib/cpptest-lite build/cpptest-lite)
    add_subdirectory(test build/test)
endif (BUILD_TESTING)
