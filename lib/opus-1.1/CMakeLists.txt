cmake_minimum_required(VERSION 2.6)

PROJECT(opus)

# message("Source Dir:" ${opus_SOURCE_DIR})

SET(EXECUTABLE_OUTPUT_PATH ${opus_BINARY_DIR})
SET(LIBRARY_OUTPUT_PATH ${opus_BINARY_DIR})
SET(RUNTIME_OUTPUT_DIRECTORY ${opus_BINARY_DIR})

SET(opus_BIN ${opus_BINARY_DIR})

# message("opus_SOURCE_DIR directories:" ${opus_SOURCE_DIR})

add_definitions(/DHAVE_CONFIG_H)

IF(MSVC)
add_definitions(-DUNICODE -D_UNICODE)
ENDIF(MSVC)

IF(WIN32)
SET(opusIncludes
${opus_SOURCE_DIR}
${opus_SOURCE_DIR}/include
${opus_SOURCE_DIR}/win32
)
ELSEIF(UNIX)
# Include directories
SET(opusIncludes
${opus_SOURCE_DIR}
${opus_SOURCE_DIR}/include
/usr/local/include
)
ENDIF(WIN32)

# message("opusIncludes directories:" ${opusIncludes})

# lib directories
IF(WIN32)
SET(opusLink
${opus_SOURCE_DIR}/lib
)
ELSEIF(UNIX)
SET(opusLink
${opus_SOURCE_DIR}/lib
/usr/local/lib
/usr/lib
)
ENDIF(WIN32)

# message("opusLink directories:" ${opusLink})

ADD_SUBDIRECTORY( celt )
ADD_SUBDIRECTORY( silk ) 
ADD_SUBDIRECTORY( src) 
