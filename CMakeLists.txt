cmake_minimum_required(VERSION 3.9)

project(CppAIKit
        VERSION 0.0.1
        DESCRIPTION "C++ Artificial Intelligence Kit - Library for AI implementation in games and other projects."
        LANGUAGES CXX)

add_subdirectory(deps)

### Library target creation and source listing

add_library(CppAIKit INTERFACE)

target_include_directories(CppAIKit INTERFACE
        $<BUILD_INTERFACE:${CppAIKit_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${CppAIKit_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

configure_file(
        "${CppAIKit_SOURCE_DIR}/include/cppaikit/Config.hpp.in"
        "${CppAIKit_BINARY_DIR}/include/cppaikit/Config.hpp")

target_compile_features(CppAIKit INTERFACE cxx_std_17)

### Options listing

option(CppAIKit_DOC "Enable doxygen documentation build" OFF)
option(CppAIKit_EXAMPLE "Enable examples build" ON)
option(CppAIKit_TEST "Enable tests" ON)

### Documentation

if (CppAIKit_DOC)
    message(STATUS "CppAIKit: Doxygen documentation enabled")
    add_subdirectory(docs/doxygen)
endif ()

### Examples

if (CppAIKit_EXAMPLE)
    message(STATUS "CppAIKit: Examples enabled")
    add_subdirectory(examples)
endif ()

### Testing

if (CppAIKit_TEST)
    message(STATUS "CppAIKit: Tests enabled")
    enable_testing(true)
    add_subdirectory(test)
endif ()

### Export library interface

add_library(CppAIKit::CppAIKit ALIAS CppAIKit)

include(GNUInstallDirs)
install(TARGETS CppAIKit EXPORT CppAIKitTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} # For Windows
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT CppAIKitTargets
        FILE CppAIKitTargets.cmake
        NAMESPACE CppAIKit::
        DESTINATION lib/cmake/cppaikit)

include(CMakePackageConfigHelpers)
write_basic_package_version_file("CppAIKitConfigVersion.cmake"
        VERSION ${CppAIKit_VERSION}
        COMPATIBILITY SameMajorVersion)

install(FILES "CppAIKitConfig.cmake" "CppAIKitConfigVersion.cmake"
        DESTINATION lib/cmake/cppaikit)
