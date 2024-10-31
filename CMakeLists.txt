# CMakeLists.txt in UserProject
cmake_minimum_required(VERSION 3.28)
project(RSA)

set(ENV{http_proxy} "http://127.0.0.1:10989")
set(ENV{https_proxy} "http://127.0.0.1:10989")

enable_testing()

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

LINK_DIRECTORIES(${PYTHON_PATH}/libs)

include(fetch-gtest)
include(fetch-boost)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

add_subdirectory(src)
add_subdirectory(test)