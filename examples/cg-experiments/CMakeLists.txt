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

function ( Kernel name )
    configure_file(${name} ${name} COPYONLY)
endfunction()


Experiment(ForwardEuler)
Experiment(ForwardEulerVector)
Experiment(RungeKutta)
Experiment(Volume)

Experiment(BandwidthForwardEuler)
Experiment(BandwidthRungeKutta)
Experiment(BandwidthVolume)

Experiment(SpeedupForwardEuler)
Experiment(SpeedupForwardEulerVector)
Experiment(SpeedupRungeKutta)
Experiment(SpeedupRungeKuttaStripped)
Experiment(SpeedupRungeKuttaHandWrittenOCL)
Experiment(SpeedupVolume)

Kernel(ForwardEuler.cl)
Kernel(ForwardEulerVector.cl)
Kernel(RungeKutta.cl)
Kernel(RungeKuttaHandWritten.cl)
Kernel(Volume.cl)
