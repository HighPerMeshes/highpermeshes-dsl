find_package(GTest REQUIRED)

add_executable ( tests
    Tests.cpp
    drts/data_flow/GraphTest.cpp
    drts/data_flow/DataDependencyMaps.cpp 
    dsl/mesh/PartitionedMesh.cpp
    dsl/loop_types/AccessDefinitionHelpers.cpp
    dsl/tmp/util/IsAccessDefinitionTest.cpp
    dsl/tmp/util/IsDataAccessesTest.cpp
    dsl/tmp/util/IsExpressionSupportedTest.cpp
    dsl/tmp/util/IsTemplateSpecialization.hpp
    dsl/tmp/util/TupleTypeTraitsTest.cpp
    dsl/dispatchers/TimeStep.cpp
)
              
target_include_directories (tests PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR} )

target_link_libraries( tests LINK_PRIVATE  
    GTest::GTest 
    GTest::Main 
    HighPerMeshes::HighPerMeshes )
