# FindMETIS.cmake
# 
# Finds the metis library
#
# This will define the following variables
#
#   METIS_FOUND
#   METIS_INCLUDE_DIRS
#   METIS_LIBARIES
#
# and the following imported targets
#
#   METIS::metis
#

# Gets the environment variable METIS_DIR if possible
if( DEFINED ENV{METIS_DIR} )
  set( METIS_DIR "$ENV{METIS_DIR}" )
endif()

# Tries to find the appropriate include path
find_path(
  METIS_INCLUDE_DIR
    metis.h
  HINTS
    ${METIS_DIR}
  PATH_SUFFIXES
    include
)

# Tries to find the appropriate library
find_library( METIS_LIBRARY
  NAMES 
    metis
  HINTS 
    ${METIS_DIR}
  PATH_SUFFIXES 
    lib
)

# Handles standard argument of the find call such as REUIRED
include( FindPackageHandleStandardArgs )
find_package_handle_standard_args( 
    METIS 
    DEFAULT_MSG
    METIS_INCLUDE_DIR
    METIS_LIBRARY
)

if( METIS_FOUND )

  # It is convention to pluralize the include and library variables
  set( METIS_INCLUDE_DIRS ${METIS_INCLUDE_DIR} )
  set( METIS_LIBRARIES ${METIS_LIBRARY} )

  # This is for the cmake-gui to not clutter options
  mark_as_advanced(
    METIS_LIBRARY
    METIS_INCLUDE_DIR
    METIS_DIR
  )

  # Add metis as an imported target
  if(NOT TARGET METIS::metis)
    add_library(METIS::metis UNKNOWN IMPORTED)
    set_target_properties(METIS::metis PROPERTIES IMPORTED_LOCATION "${METIS_LIBRARIES}")
    set_target_properties(METIS::metis PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${METIS_INCLUDE_DIRS}")
  endif()
  
endif()