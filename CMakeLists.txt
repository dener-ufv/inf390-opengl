cmake_minimum_required(VERSION 3.10)
project(opengl VERSION 1.0)

# project
add_executable(opengl src/main.cpp)
target_include_directories(opengl PUBLIC include/)

# glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/glfw)
target_link_libraries(opengl glfw)

# opengl
find_package(OpenGL REQUIRED)
target_link_libraries(opengl OpenGL::GL)

# glad
add_subdirectory(lib/glad)
target_link_libraries(opengl glad)

# glm
add_subdirectory(lib/glm)
target_link_libraries(opengl glm)
