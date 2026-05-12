cmake_minimum_required(VERSION 3.15)
project(Atoms LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

# Your source files
file(GLOB SRC_FILES "src/*.cpp")

# ImGui files
file(GLOB IMGUI_SRC
    "src/imgui/*.cpp"
    "src/imgui/backends/imgui_impl_glfw.cpp"
    "src/imgui/backends/imgui_impl_opengl3.cpp"
)

# 🚨 ONLY ONE executable
add_executable(Atoms
    ${SRC_FILES}
    ${IMGUI_SRC}
)

if (MSVC)
    target_compile_options(Atoms PRIVATE /FS)
endif()

# Includes
target_include_directories(Atoms PRIVATE
    src
    src/imgui
    src/imgui/backends
)

# Link libs
target_link_libraries(Atoms PRIVATE
    OpenGL::GL
    GLEW::GLEW
    glfw
    glm::glm
)