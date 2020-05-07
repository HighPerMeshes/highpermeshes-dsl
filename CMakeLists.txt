cmake_minimum_required (VERSION 3.1)
project (HighPerMeshes)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_BUILD_TYPE Release)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -ftree-vectorize -ffast-math -Wall -Wextra -pedantic")
set (BUILD_EXAMPLES on)
set (BUILD_TESTS off)

# Create a library called "HighPerMeshes", also add an alias for the correct namespace
add_library ( HighPerMeshes INTERFACE)
add_library( HighPerMeshes::HighPerMeshes ALIAS HighPerMeshes)

# HighPerMeshes required Metis for linking
target_link_libraries(HighPerMeshes INTERFACE METIS::metis)

# Define headers for this library. 
target_include_directories(HighPerMeshes 
	INTERFACE 
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
   )

include(GNUInstallDirs)

# Where to install the configuration for the HighPerMeshes package 
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/HighPerMeshes)

# How to install targets
install(TARGETS HighPerMeshes 
	EXPORT HighPerMeshes-Targets
	ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})  # This is for Windows

# How to install headers
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Export targets to a script  
install(
	EXPORT HighPerMeshes-Targets
	FILE HighPerMeshesTargets.cmake
	NAMESPACE HighPerMeshes::
	DESTINATION ${INSTALL_CONFIGDIR}
)

include(CMakePackageConfigHelpers)
configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/HighPerMeshesConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/HighPerMeshesConfig.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

# Install config and find modules
install(FILES
    ${CMAKE_CURRENT_LIST_DIR}/cmake/FindMETIS.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/HighPerMeshesConfig.cmake
    DESTINATION ${INSTALL_CONFIGDIR}
)

configure_file(
	${CMAKE_CURRENT_LIST_DIR}/cmake/FindMETIS.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/FindMETIS.cmake
    COPYONLY)

export(
	EXPORT HighPerMeshes-Targets 
	FILE ${CMAKE_CURRENT_BINARY_DIR}/HighPerMeshesTargets.cmake 
	NAMESPACE HighPerMeshes::
)

# Register package
export(PACKAGE HighPerMeshes)

if(BUILD_TESTS OR BUILD_EXAMPLES)
  find_package(METIS REQUIRED)
endif()

if (BUILD_TESTS)
  	add_subdirectory (tests)
endif (BUILD_TESTS)

if (BUILD_EXAMPLES)
	add_subdirectory (examples)
endif (BUILD_EXAMPLES)
