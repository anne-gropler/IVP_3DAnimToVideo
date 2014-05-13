# Find libcgs
#
# This module defines
# LIBCGS_FOUND:         true if found
# LIBCGS_CMAKE_COMMONS: Where to find 'common.cmake'
# LIBCGS_INCLUDE_DIRS:  Where to find the header files
# LIBCGS_LIBRARIES:     Where to find the (static) libraries
#
# And for each submodule:
# LIBCGS_<NAME>_LIBRARY_DEBUG
# LIBCGS_<NAME>_LIBRARY_RELEASE
# LIBCGS_<NAME>_LIBRARY


############################################################
# MACRO add_library_for_config
#
# Add a library for a certain configuration, e.g. 'debug' or 'optimized'.
# The macro checks if $input is set, if yes, it adds it to the output variable
# with the given prefix. If $input is not set, but $fallback is set, $fallback is
# used instead, e.g. to allow for debug libraries to be used if the release version is
# not available or vice versa.
#
# example call: add_library_for_config(MY_LIBRARY optimized MY_LIBRARY_RELEASE MY_LIBRARY_DEBUG)
#
macro(add_library_for_config dest config lib fallback)
	if(NOT ${dest})
		set(${dest} "")
	endif()
    if(${lib})
        set(${dest} ${${dest}} ${config} ${${lib}})
    elseif(${fallback})
        set(${dest} ${${dest}} ${config} ${${fallback}})
    endif()
	if(NOT ${dest})
		set(${dest} "${dest}-NOTFOUND")
	endif()
endmacro()

############################################################
# MACRO find_libcgs_library
#
# Find libcgs library
#
# example call: find_libcgs_library(LIBCGS_UTIL_LIBRARY cgsutil)
#
macro(find_libcgs_library dest name)
	set(${dest} "")
	find_library(${dest}_RELEASE  NAMES ${name}        PATHS ${LIBCGS_ROOT_DIR}/lib)
	find_library(${dest}_DEBUG    NAMES ${name}_debug  PATHS ${LIBCGS_ROOT_DIR}/lib)
	add_library_for_config(${dest}  optimized ${dest}_RELEASE ${dest}_DEBUG)
	add_library_for_config(${dest}  debug     ${dest}_DEBUG   ${dest}_RELEASE)
    set(${dest} "${${dest}}" CACHE STRING "Path to ${name} library")
    if(${dest})
		message(STATUS "Found ${name}: ${${dest}}")
	else()
		message(STATUS "${name} NOT found.")
	endif()
    if(${dest})
		set(LIBCGS_LIBRARIES ${LIBCGS_LIBRARIES} ${${dest}})
	endif()
endmacro()


# Find libcgs root directory
find_path(LIBCGS_ROOT_DIR cmake/common.cmake
    ${CGS_ROOT_DIR}
    $ENV{CGS_REPOSITORY_ROOT}
    ${CMAKE_SOURCE_DIR}/../libcgs
    ${CMAKE_SOURCE_DIR}/../../libcgs
)

# Set common cmake include
set(LIBCGS_CMAKE_COMMONS ${LIBCGS_ROOT_DIR}/cmake/common.cmake)

# Find include files
find_path(LIBCGS_INCLUDE_DIRS cgsdb/Object.h ${LIBCGS_ROOT_DIR}/include)

set(LIBCGS_LIBRARIES "")

# Find libcgs libraries
find_libcgs_library(LIBCGS_DB_LIBRARY          cgsdb)
find_libcgs_library(LIBCGS_GUI_LIBRARY         cgsgui)
find_libcgs_library(LIBCGS_IMG_LIBRARY         cgsimg)
find_libcgs_library(LIBCGS_INTERACTION_LIBRARY cgsinteraction)
find_libcgs_library(LIBCGS_IO_LIBRARY          cgsio)
find_libcgs_library(LIBCGS_NET_LIBRARY         cgsnet)
find_libcgs_library(LIBCGS_SCENE_LIBRARY       cgsscene)
find_libcgs_library(LIBCGS_UTIL_LIBRARY        cgsutil)
find_libcgs_library(LIBCGS_CLUSTER_LIBRARY     cgscluster)

# Put library list together
if(NOT LIBCGS_LIBRARIES)
	set(LIBCGS_LIBRARIES "LIBCGS_LIBRARIES-NOTFOUND")
endif()

# Check if library has been found
set(LIBCGS_FOUND "NO")
if(LIBCGS_LIBRARIES AND LIBCGS_INCLUDE_DIRS)
    set(LIBCGS_FOUND "YES")
endif()

# [DEBUG]
#message("LIBCGS_LIBRARIES: ${LIBCGS_LIBRARIES}")
