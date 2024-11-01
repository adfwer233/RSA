include(FetchContent)

set(CMAKE_CXX_STANDARD 20)

FetchContent_Declare(
        pybind11
        GIT_REPOSITORY https://github.com/pybind/pybind11.git
        GIT_TAG        v2.13.6
)

FetchContent_MakeAvailable(pybind11)