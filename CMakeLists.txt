cmake_minimum_required(VERSION 3.10.0)
project(YADRO-test-task-base-stations-SHD VERSION 0.1.0 LANGUAGES C CXX)

add_executable(YADRO-test-task 
src/main.cpp
)

include(CTest)
enable_testing()

