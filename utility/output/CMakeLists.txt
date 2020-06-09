cmake_minimum_required (VERSION 3.1)
project (Parser)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake)

set (CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED)

add_executable(parser parser.cpp)

target_include_directories(parser PRIVATE include)

option(BUILD_TESTS "Build Tests?" off)

if(BUILD_TESTS) 
  add_subdirectory(tests)
endif()
