cmake_minimum_required(VERSION 3.15)
project(Atoms LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

# Gather all cpp files in src/ automatically
file(GLOB SRC_FILES "src/*.cpp")

# Create one executable per source file
foreach(SRC ${SRC_FILES})
    # Get filename without extension
    get_filename_component(TARGET_NAME ${SRC} NAME_WE)
    add_executable(${TARGET_NAME} ${SRC})

    # Link libraries
    target_link_libraries(${TARGET_NAME} PRIVATE OpenGL::GL GLEW::GLEW glfw glm::glm)
endforeach()