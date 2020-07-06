find_package(Catch2 REQUIRED)

add_executable(tests tests.cpp)
target_link_libraries(tests Catch2::Catch2)

target_include_directories(tests PRIVATE . ../include)
