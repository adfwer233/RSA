project(rsa_benchmark VERSION 1.0 DESCRIPTION "RSA Benchmark Project" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

set(SRC_FILE
    rsa_benchmark.cpp
)

add_executable(${PROJECT_NAME} ${SRC_FILE})

target_link_libraries(${PROJECT_NAME} benchmark::benchmark)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)