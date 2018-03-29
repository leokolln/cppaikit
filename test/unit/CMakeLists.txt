file(GLOB_RECURSE TEST_SOURCES *.cpp)

set(TEST_NAME CppAIKit-test)

add_executable(${TEST_NAME} ${TEST_SOURCES})
target_link_libraries(${TEST_NAME} CppAIKit::CppAIKit catch)

add_test(NAME UnitTest COMMAND CppAIKit-test)
