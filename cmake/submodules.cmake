#
# Originally taken from:
# https://github.com/xdbr/cmake-module-submodule.cmake
#
# All credits to the original author!
#

if(EXISTS "${PROJECT_SOURCE_DIR}/.gitmodules")
message(STATUS "Updating submodules to their latest/fixed versions")
message(STATUS "(this can take a while, please be patient)")

### set the direcory where the submodules live
set(GIT_SUBMODULES_DIRECTORY lib)

### set the directory names of the submodules
set(GIT_SUBMODULES cpptest-lite rtaudio opus)

### set each submodules's commit or tag that is to be checked out
### (leave empty if you want master)
set(GIT_SUBMODULE_VERSION_cpptest-lite stable)
set(GIT_SUBMODULE_VERSION_rtaudio 4.1.2)
set(GIT_SUBMODULE_VERSION_opus v1.1.2)

### First, get all submodules in
if(${GIT_SUBMODULES_CHECKOUT_QUIET})
    execute_process(
        COMMAND             git submodule update --init --recursive
        WORKING_DIRECTORY   ${PROJECT_SOURCE_DIR}
        OUTPUT_QUIET
        ERROR_QUIET
    )
else()
    execute_process(
        COMMAND             git submodule update --init --recursive
        WORKING_DIRECTORY   ${PROJECT_SOURCE_DIR}
    )
endif()

### Then, checkout each submodule to the specified commit
# Note: Execute separate processes here, to make sure each one is run,
# should one crash (because of branch not existing, this, that ... whatever)
foreach(GIT_SUBMODULE ${GIT_SUBMODULES})

    if( "${GIT_SUBMODULE_VERSION_${GIT_SUBMODULE}}" STREQUAL "" )
        message(STATUS "no specific version given for submodule ${GIT_SUBMODULE}, checking out master")
        set(GIT_SUBMODULE_VERSION_${GIT_SUBMODULE} "master")
    endif()

    if(${GIT_SUBMODULES_CHECKOUT_QUIET})
        execute_process(
            COMMAND             git checkout ${GIT_SUBMODULE_VERSION_${GIT_SUBMODULE}}
            WORKING_DIRECTORY   ${PROJECT_SOURCE_DIR}/${GIT_SUBMODULES_DIRECTORY}/${GIT_SUBMODULE}
            OUTPUT_QUIET
            ERROR_QUIET
        )
    else()
        message(STATUS "checking out ${GIT_SUBMODULE}'s commit/tag ${GIT_SUBMODULE_VERSION_${GIT_SUBMODULE}}")
        execute_process(
            COMMAND             git checkout ${GIT_SUBMODULE_VERSION_${GIT_SUBMODULE}}
            WORKING_DIRECTORY   ${PROJECT_SOURCE_DIR}/${GIT_SUBMODULES_DIRECTORY}/${GIT_SUBMODULE}
        )
    endif()

endforeach(${GIT_SUBMODULE})

endif()
