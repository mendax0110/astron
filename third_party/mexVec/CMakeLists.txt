cmake_minimum_required(VERSION 3.20)
project(mexVec VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME}
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_23)

install(FILES include/Vec.hpp
        DESTINATION include/mex
)

install(TARGETS ${PROJECT_NAME}
        EXPORT mexVecTargets
)

install(EXPORT mexVecTargets
        FILE mexVecTargets.cmake
        NAMESPACE mex::
        DESTINATION lib/cmake/mexVec
)

option(BUILD_TESTING "Build tests" ON)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

message(STATUS "")
message(STATUS "===========================================")
message(STATUS "mexVec Configuration")
message(STATUS "===========================================")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Testing: ${BUILD_TESTING}")
message(STATUS "===========================================")