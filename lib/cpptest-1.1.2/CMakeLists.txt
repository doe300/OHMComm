cmake_minimum_required (VERSION 2.6)
project (CppTest)

# Include the source files
file( GLOB SRCS src/*.cpp src/*.h)
file( GLOB WIN win/winconfig.h)

# set output paths
#SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/build/cpptest)
#SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/build/cpptest)


# include headers in the project settings (as search-path for header-files)
include_directories ("${PROJECT_SOURCE_DIR}/src")

if(WIN32)
include_directories ("${PROJECT_SOURCE_DIR}/win")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN32_WINNT=0x0501")
add_definitions(-D__WIN32__)
add_definitions(-DWIN32)
endif()



add_library(CppTest STATIC ${SRCS} ${WIN})
