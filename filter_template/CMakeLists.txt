# Arbitrary version number. Unclear what I actually need...
cmake_minimum_required(VERSION 3.10)

project(filter_template VERSION 1.0.1 DESCRIPTION "template for HDF5 filters")

include(GNUInstallDirs)

#------------------------------------------------------------------------------
# Add the filter plugin
#------------------------------------------------------------------------------
add_library(filter_template SHARED
    filter_template.c
)

#------------------------------------------------------------------------------
# Add the test program
#------------------------------------------------------------------------------
add_executable(filter_test_program
    filter_test_program.c
)
# Copy the shell script that makes it obvious you need to set the plugin path
add_custom_command(
    TARGET filter_test_program POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/runme.sh
            ${CMAKE_CURRENT_BINARY_DIR}/runme.sh
)

#------------------------------------------------------------------------------
# Set a default build type if none was specified
#------------------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

#------------------------------------------------------------------------------
# Find HDF5
#------------------------------------------------------------------------------
# You probably only need 1.8 for this to work...
find_package(HDF5 NO_MODULE NAMES hdf5 COMPONENTS C shared)
if(HDF5_FOUND)
    set(HDF5_C_SHARED_LIBRARY hdf5-shared)
    if(NOT TARGET ${HDF5_C_SHARED_LIBRARY})
        message(FATAL_ERROR "Could not find hdf5 shared target, please make "
        "sure that HDF5 has ben compiled with shared libraries enabled.")
    endif()
    set(FILTER_TEMPLATE_EXT_PKG_DEPENDENCIES
        ${FILTER_TEMPLATE_EXT_PKG_DEPENDENCIES}
        ${HDF5_C_SHARED_LIBRARY})
else()
    # Allow for HDF5 autotools builds
    # NOTE: I have not gotten this to work...
    find_package(HDF5 MODULE REQUIRED)
    if(HDF5_FOUND)
        set(FILTER_TEMPLATE_EXT_INCLUDE_DEPENDENCIES
            ${FILTER_TEMPLATE_EXT_INCLUDE_DEPENDENCIES}
            ${HDF5_INCLUDE_DIRS})
        set(FILTER_TEMPLATE_EXT_LIB_DEPENDENCIES
            ${FILTER_TEMPLATE_EXT_LIB_DEPENDENCIES}
            ${HDF5_LIBRARIES})
    else()
        message(FATAL_ERROR "Could not find HDF5, please check HDF5_DIR.")
    endif()
endif()

#------------------------------------------------------------------------------
# Some minimum target properties
#------------------------------------------------------------------------------
set_target_properties(filter_template PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER filter_template.h
)

#------------------------------------------------------------------------------
# Set external include directories and libraries
#------------------------------------------------------------------------------
target_include_directories(filter_template
    SYSTEM PUBLIC ${FILTER_TEMPLATE_EXT_INCLUDE_DEPENDENCIES}
)
target_link_libraries(filter_template
    ${FILTER_TEMPLATE_EXT_LIB_DEPENDENCIES}
    ${FILTER_TEMPLATE_EXT_PKG_DEPENDENCIES}
)

target_include_directories(filter_test_program
    SYSTEM PUBLIC ${FILTER_TEMPLATE_EXT_INCLUDE_DEPENDENCIES}
)
target_link_libraries(filter_test_program
    ${FILTER_TEMPLATE_EXT_LIB_DEPENDENCIES}
    ${FILTER_TEMPLATE_EXT_PKG_DEPENDENCIES}
)

#------------------------------------------------------------------------------
# Install stuff
#------------------------------------------------------------------------------
install(TARGETS filter_template
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

