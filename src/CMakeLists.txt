project(RSADemo VERSION 1.0 DESCRIPTION "Graphics Lab Main Application" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

set(SRC_FILE main.cpp)

add_executable(${PROJECT_NAME} ${SRC_FILE})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)

pybind11_add_module(rsa_py binding.cpp)
target_include_directories(rsa_py PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Define the destination directory for the post-build copy
set(DEST_DIR "${CMAKE_SOURCE_DIR}/demo")

# Add a post-build command to copy the generated files to the destination directory
add_custom_command(TARGET rsa_py POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:rsa_py> ${DEST_DIR}
        COMMENT "Copying rsa_py module to ${DEST_DIR}"
)
