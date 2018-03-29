# Create a target for Catch test library
project(catch LANGUAGES CXX)

add_library(catch INTERFACE)
target_include_directories(catch INTERFACE
        $<BUILD_INTERFACE:${catch_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${catch_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

### Compilation options, features and definitions
target_compile_features(catch INTERFACE cxx_std_11)

### Export library interface
include(GNUInstallDirs)
install(TARGETS catch EXPORT catchTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} # This is for Windows
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT catchTargets
        FILE catchTargets.cmake
        NAMESPACE catch::
        DESTINATION lib/cmake/catch)
