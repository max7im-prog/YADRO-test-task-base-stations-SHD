cmake_minimum_required(VERSION 3.10.0)
project(YADRO-test-task-base-stations-SHD VERSION 0.1.0 LANGUAGES C CXX)

# Main executable
add_executable(YADRO-test-task 
    src/main.cpp
    src/club.h
    src/club.cpp
)

include(CTest)
enable_testing()


add_subdirectory(external/googletest)

# Test executable
add_executable(tests_runner
    test/test_basic.cpp
    src/club.h
    src/club.cpp
)

target_link_libraries(tests_runner
    gtest_main

)

include(GoogleTest)
gtest_discover_tests(tests_runner)