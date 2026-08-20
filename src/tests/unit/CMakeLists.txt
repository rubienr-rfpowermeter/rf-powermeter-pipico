cmake_minimum_required(VERSION 3.16)

project(rf_probe_tests)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(googletest
        URL https://github.com/google/googletest/releases/download/v1.18.0/googletest-1.18.0.tar.gz
        URL_HASH SHA256=6e3191c1455468b3fc35a417fb565c1c5071aee1b7e7f85e30cf48a98d37d8b5
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
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

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(tests PRIVATE
            -fsanitize=address,undefined
            -fno-sanitize-recover=undefined
            -fno-omit-frame-pointer
    )
    target_link_options(tests PRIVATE
            -fsanitize=address,undefined
            -fno-sanitize-recover=undefined
    )
else ()
    message(FATAL_ERROR "The unit tests require a compiler with AddressSanitizer and UndefinedBehaviorSanitizer support")
endif ()

target_link_libraries(tests
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(tests)
