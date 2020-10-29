find_package(OpenMP REQUIRED)

function( Experiment name )
    
    add_executable( ${name}.out ${name}.cpp )

    target_link_libraries(
        ${name}.out
        LINK_PRIVATE
        HighPerMeshes::HighPerMeshes
        OpenMP::OpenMP_CXX
    )


    target_link_libraries (${name}.out PRIVATE -pthread -lOpenCL)

    target_include_directories(
        ${name}.out
        PRIVATE
        ../../tests/util/
    )    
endfunction()

Experiment(Experiments)