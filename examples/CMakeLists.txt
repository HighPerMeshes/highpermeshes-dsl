find_package(OpenMP REQUIRED)

add_executable( midg2
    ./MIDG2_DSL/mainUnfusedKernels.cpp )	
set_target_properties(midg2 PROPERTIES COMPILE_FLAGS "-save-temps") 	
target_link_libraries (midg2 LINK_PRIVATE HighPerMeshes::HighPerMeshes OpenMP::OpenMP_CXX)

add_executable(minimal MinimalWorkingExample.cpp)
target_link_libraries (minimal LINK_PRIVATE HighPerMeshes::HighPerMeshes OpenMP::OpenMP_CXX)
target_include_directories(minimal PRIVATE ../tests/util/)

add_executable(experiments Experiments.cpp)
target_link_libraries (experiments LINK_PRIVATE HighPerMeshes::HighPerMeshes OpenMP::OpenMP_CXX)
target_include_directories(experiments PRIVATE ../tests/util/)

add_executable(writeLoopExample WriteLoopExample.cpp)
target_link_libraries (writeLoopExample LINK_PRIVATE HighPerMeshes::HighPerMeshes OpenMP::OpenMP_CXX)
target_include_directories(writeLoopExample PRIVATE ../tests/util/ ../utility/output)

configure_file(./MIDG2_DSL/config.cfg config.cfg COPYONLY)
configure_file(./MIDG2_DSL/F072.neu F072.neu COPYONLY)
