cmake_minimum_required(VERSION 3.25)

project(astron VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets OpenGLWidgets)
find_package(VTK 9.4 REQUIRED COMPONENTS
    CommonCore
    CommonDataModel
    FiltersCore
    GUISupportQt
    InteractionStyle
    RenderingCore
    RenderingOpenGL2
)

if(NOT VTK_QT_VERSION STREQUAL "6")
    message(FATAL_ERROR "Astron requires VTK built against Qt 6. Found VTK_QT_VERSION=${VTK_QT_VERSION}.")
endif()
find_package(PkgConfig REQUIRED)
pkg_check_modules(NETCDF REQUIRED IMPORTED_TARGET netcdf)

add_library(astron_domain STATIC
    src/core/equilibrium.cpp
    src/core/equilibrium.h
    src/io/vmec_netcdf_reader.cpp
    src/io/vmec_netcdf_reader.h
    src/jobs/vmec_runner.cpp
    src/jobs/vmec_runner.h
    src/wrapper/netcdf_file_wrapper.cpp
    src/wrapper/netcdf_file_wrapper.h
    src/simulation/particle.h
    src/simulation/boris_integrator.cpp
    src/simulation/boris_integrator.h
    src/simulation/field_provider.h
)
target_include_directories(astron_domain PUBLIC src)
target_link_libraries(astron_domain PUBLIC Qt6::Core PkgConfig::NETCDF)

qt_add_executable(astron
    src/app/main.cpp
    src/visualization/stellarator_geometry.cpp
    src/visualization/stellarator_geometry.h
    src/ui/main_window.cpp
    src/ui/main_window.h
)

target_link_libraries(astron PRIVATE
    astron_domain
    Qt6::Widgets
    Qt6::OpenGLWidgets
    ${VTK_LIBRARIES}
)

find_package(Python3 COMPONENTS Interpreter QUIET)
if(Python3_Interpreter_FOUND)
    add_custom_target(check-namespace-style
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/check_namespace_style.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Checking namespace declarations"
    )
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(astron_domain PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(astron PRIVATE -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(astron_domain PRIVATE /W4 /permissive-)
    target_compile_options(astron PRIVATE /W4 /permissive-)
endif()

vtk_module_autoinit(TARGETS astron MODULES ${VTK_LIBRARIES})

option(ASTRON_BUILD_TESTS "Build Astron unit tests" ON)
if(ASTRON_BUILD_TESTS)
    enable_testing()
    find_package(GTest QUIET)
    if(NOT GTest_FOUND)
        include(FetchContent)
        set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
        FetchContent_Declare(googletest
            URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        )
        FetchContent_MakeAvailable(googletest)
    endif()

    add_executable(astron_tests
        tests/main.cpp
        tests/equilibrium_test.cpp
        tests/vmec_netcdf_reader_test.cpp
        tests/vmec_runner_test.cpp
        tests/particle_integrator_test.cpp
    )
    target_link_libraries(astron_tests PRIVATE astron_domain GTest::gtest)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(astron_tests PRIVATE -Wall -Wextra -Wpedantic)
    elseif(MSVC)
        target_compile_options(astron_tests PRIVATE /W4 /permissive-)
    endif()

    include(GoogleTest)
    gtest_discover_tests(astron_tests DISCOVERY_TIMEOUT 30)
endif()
