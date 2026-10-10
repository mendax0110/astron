cmake_minimum_required(VERSION 3.14)

include(FetchContent)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
        googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.8.3
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_ASSEMBLY_TESTS OFF CACHE BOOL "" FORCE)
set(BENCHMARK_USE_BUNDLED_GTEST OFF CACHE BOOL "" FORCE)
set(HAVE_STD_REGEX OFF CACHE BOOL "" FORCE)
set(HAVE_POSIX_REGEX OFF CACHE BOOL "" FORCE)
set(HAVE_GNU_POSIX_REGEX OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googlebenchmark)

add_executable(vec_test vec_test.cpp)

target_link_libraries(vec_test
        PRIVATE
        mexVec
        GTest::gtest
        GTest::gtest_main
)

add_executable(vec_benchmark vec_benchmark_simple.cpp)

target_link_libraries(vec_benchmark
        PRIVATE
        mexVec
        benchmark::benchmark
        benchmark::benchmark_main
)

include(GoogleTest)
gtest_discover_tests(vec_test)

add_custom_target(run_tests
        COMMAND vec_test --gtest_color=yes
        DEPENDS vec_test
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running unit tests..."
        VERBATIM
)

add_custom_target(run_benchmarks
        COMMAND vec_benchmark --benchmark_format=console
        DEPENDS vec_benchmark
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running benchmarks..."
        VERBATIM
)

add_custom_target(run_all
        DEPENDS vec_test vec_benchmark
        COMMENT "Building all tests and benchmarks..."
)

message(STATUS "Tests configured:")
message(STATUS "  vec_test        - Unit tests with Google Test")
message(STATUS "  vec_benchmark   - Performance benchmarks")
message(STATUS "  run_tests       - Build and run unit tests")
message(STATUS "  run_benchmarks  - Build and run benchmarks")