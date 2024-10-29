project(rsa_test VERSION 1.0 DESCRIPTION "Test Project" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

set(SRC_FILE
        main.cpp
        simple_test.cpp
)

enable_testing()

add_executable(${PROJECT_NAME} ${SRC_FILE})

target_link_libraries(${PROJECT_NAME} GTest::gtest_main GTest::gtest)

include(GoogleTest)
gtest_discover_tests(rsa_test)