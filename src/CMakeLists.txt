#Include headers in the project structure
file( GLOB HDRS "${PROJECT_SOURCE_DIR}/include/*.h" )

#Include headers in the project settings (as search-path for header-files)
include_directories ("${PROJECT_SOURCE_DIR}/include")

#Include the source files
file( GLOB SRCS *.cpp )

#Build OHMComm
add_executable( OHMComm ${SRCS} ${HDRS} )
target_link_libraries(OHMComm rtaudio)
