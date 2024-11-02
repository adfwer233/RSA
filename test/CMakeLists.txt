project(rsa_test VERSION 1.0 DESCRIPTION "Test Project" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

set(SRC_FILE
        main.cpp
        rsa_test.cpp
        simple_test.cpp
        integer_test.cpp
        prime_generator_test.cpp
)

enable_testing()

add_executable(${PROJECT_NAME} ${SRC_FILE})

target_link_libraries(${PROJECT_NAME} spdlog GTest::gtest_main GTest::gtest)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)

include(GoogleTest)
gtest_discover_tests(rsa_test)