cmake_minimum_required(VERSION 3.9)

project(CppAIKit VERSION 0.0.1 LANGUAGES CXX)

add_subdirectory(deps)

### Library target creation and source listing

add_library(CppAIKit
        src/main.cpp
        include/cppaikit/fsm/FSM.hpp
        include/cppaikit/fsm/State.hpp)

target_include_directories(CppAIKit PUBLIC
        $<BUILD_INTERFACE:${CppAIKit_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${CppAIKit_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

### Compilation options, features and definitions

target_compile_features(CppAIKit PUBLIC cxx_std_17)

if (MSVC)
    target_compile_options(CppAIKit PRIVATE
            /FAs /W4 /WX
            /wd4309 /wd4324 /wd4389 /wd4127 /wd4267 /wd4146
            /wd4201 /wd4464 /wd4514 /wd4701 /wd4820 /wd4365)
    target_compile_definitions(CppAIKit PRIVATE _CRT_SECURE_NO_WARNINGS)
else ()
    target_compile_options(CppAIKit PRIVATE
            -Wall -Wextra -Wpedantic
            -Wcast-align -Wconversion -Wold-style-cast -Wsign-conversion
            -Wredundant-decls -Wunused -Wno-missing-braces -Wshadow
            -Woverloaded-virtual -Wctor-dtor-privacy -Wnon-virtual-dtor
            -Winit-self -Werror=return-type -Wmissing-declarations -Wzero-as-null-pointer-constant -Wdouble-promotion
            -fno-exceptions -fno-strict-aliasing -fvisibility=hidden)
endif ()

### Options listing

#option(CppAIKit_DOC "Enable doxygen documentation build" ON)
option(CppAIKit_EXAMPLE "Enable examples build" ON)
option(CppAIKit_TEST "Enable tests" ON)

### Documentation

#if (CppAIKit_DOC)
#    message(STATUS "CppAIKit: Doxygen documentation enabled")
#    add_subdirectory(doc)
#endif ()

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

### Generate header file with lib version

configure_file(
        "${PROJECT_SOURCE_DIR}/include/cppaikit/Config.hpp.in"
        "${PROJECT_BINARY_DIR}/include/cppaikit/Config.hpp")

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
