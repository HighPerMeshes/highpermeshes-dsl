add_executable( midg2
    ./MIDG2_DSL/mainUnfusedKernels.cpp )		
target_link_libraries (midg2 LINK_PRIVATE HighPerMeshes::HighPerMeshes)

add_executable(minimal MinimalWorkingExample.cpp)
target_link_libraries (minimal LINK_PRIVATE HighPerMeshes::HighPerMeshes)

configure_file(./MIDG2_DSL/config.cfg config.cfg COPYONLY)
configure_file(./MIDG2_DSL/F072.neu F072.neu COPYONLY)
