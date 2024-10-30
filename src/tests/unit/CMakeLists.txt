cmake_minimum_required(VERSION 3.14)

project(rf_probe_tests)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

FetchContent_MakeAvailable(googletest)

enable_testing()

include_directories(
        ./
        ../../
)

add_executable(tests
    sampling/average.cpp
)
target_link_libraries(tests
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(tests)
