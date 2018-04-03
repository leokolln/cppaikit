find_package(Doxygen REQUIRED dot)

doxygen_add_docs(CppAIKit-doc
        include
        src
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})