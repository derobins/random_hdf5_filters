# Arbitrary version number. Unclear what I actually need...
cmake_minimum_required(VERSION 3.10)

project(shuffle VERSION 1.0.1 DESCRIPTION "shuffle filter for HDF5")

include(GNUInstallDirs)

#------------------------------------------------------------------------------
# Add the filter plugins
#------------------------------------------------------------------------------
add_library(shuffle SHARED
    shuffle.c
)

add_library(shuffle_noduff SHARED
    shuffle_noduff.c
)

add_library(shuffle_noduff_omp SHARED
    shuffle_noduff_omp.c
)

#------------------------------------------------------------------------------
# Add the test program
#------------------------------------------------------------------------------
add_executable(shuffle_test_program
    shuffle_test_program.c
)

#------------------------------------------------------------------------------
# Copy the profiling shell script
#------------------------------------------------------------------------------
add_custom_command(
    TARGET shuffle_test_program POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/profile.sh
            ${CMAKE_CURRENT_BINARY_DIR}/profile.sh
)

#------------------------------------------------------------------------------
# Set a default build type if none was specified
# This is profiling code, so it's best to build Release by default
#------------------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

#------------------------------------------------------------------------------
# Find OpenMP
#------------------------------------------------------------------------------
find_package(OpenMP REQUIRED)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp")

#------------------------------------------------------------------------------
# Find HDF5
#------------------------------------------------------------------------------
# You probably only need 1.8 for this to work...
find_package(HDF5 NO_MODULE NAMES hdf5 COMPONENTS C shared REQUIRED)
if(HDF5_FOUND)
    # NOTE: This may be hdf5-shared. Need to hack this to work with both.
    set(HDF5_C_SHARED_LIBRARY "hdf5::hdf5-shared")
    if(NOT TARGET ${HDF5_C_SHARED_LIBRARY})
        message(FATAL_ERROR "Could not find hdf5-shared target. Please make sure that HDF5 has been compiled with shared libraries enabled.")
    endif()
    set(FILTER_EXT_PKG_DEPENDENCIES
        ${FILTER_EXT_PKG_DEPENDENCIES}
        ${HDF5_C_SHARED_LIBRARY})
endif()

#------------------------------------------------------------------------------
# Some minimum target properties
#------------------------------------------------------------------------------
set_target_properties(shuffle PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER shuffle.h
)

set_target_properties(shuffle_noduff PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

set_target_properties(shuffle_noduff_omp PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

#------------------------------------------------------------------------------
# Set external include directories and libraries
#------------------------------------------------------------------------------
target_include_directories(shuffle
    SYSTEM PUBLIC ${FILTER_EXT_INCLUDE_DEPENDENCIES}
)
target_link_libraries(shuffle
    ${FILTER_EXT_LIB_DEPENDENCIES}
    ${FILTER_EXT_PKG_DEPENDENCIES}
)

target_include_directories(shuffle_noduff
    SYSTEM PUBLIC ${FILTER_EXT_INCLUDE_DEPENDENCIES}
)
target_link_libraries(shuffle_noduff
    ${FILTER_EXT_LIB_DEPENDENCIES}
    ${FILTER_EXT_PKG_DEPENDENCIES}
)

target_include_directories(shuffle_noduff_omp
    SYSTEM PUBLIC ${FILTER_EXT_INCLUDE_DEPENDENCIES}
)
target_link_libraries(shuffle_noduff_omp
    ${FILTER_EXT_LIB_DEPENDENCIES}
    ${FILTER_EXT_PKG_DEPENDENCIES}
)

target_include_directories(shuffle_test_program
    SYSTEM PUBLIC ${FILTER_EXT_INCLUDE_DEPENDENCIES}
)
target_link_libraries(shuffle_test_program
    ${FILTER_EXT_LIB_DEPENDENCIES}
    ${FILTER_EXT_PKG_DEPENDENCIES}
)

#------------------------------------------------------------------------------
# Install stuff
#------------------------------------------------------------------------------
install(TARGETS shuffle
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(TARGETS shuffle_noduff
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(TARGETS shuffle_noduff_omp
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

