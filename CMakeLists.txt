# CMakeLists.txt in UserProject
cmake_minimum_required(VERSION 3.28)
project(RSA)

enable_testing()

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

LINK_DIRECTORIES(${PYTHON_PATH}/libs)

find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

include(fetch-gtest)
include(fetch-boost)
include(fetch-pybind)
include(fetch-spdlog)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${spdlog_SOURCE_DIR}/include)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mavx -fPIC -g")
    endif()
endif()


add_subdirectory(src)
add_subdirectory(test)