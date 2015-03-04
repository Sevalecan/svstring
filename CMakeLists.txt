cmake_minimum_required(VERSION 3.0)
project(svstring_tests CXX)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(test_format tests/test_format.cpp)
set_target_properties(test_format PROPERTIES COMPILE_FLAGS -std=c++11)

