cmake_minimum_required(VERSION 3.10.0)
project(YADRO-test-task-base-stations-SHD VERSION 0.1.0 LANGUAGES C CXX)

# Set include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Create a library for your core functionality
add_library(club_lib
    src/club.cpp
)

# Main executable
add_executable(YADRO-test-task 
    src/main.cpp
)
target_link_libraries(YADRO-test-task club_lib)

# Testing setup
include(CTest)
enable_testing()

# GoogleTest
add_subdirectory(external/googletest)

# Test executable
add_executable(tests_runner
    test/test_basic.cpp
)
target_link_libraries(tests_runner
    club_lib
    gtest_main
)

# Make sure tests can find the headers
target_include_directories(tests_runner PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

include(GoogleTest)
gtest_discover_tests(tests_runner)