cmake_minimum_required(VERSION 3.24)
project(world_generator C)

set(CMAKE_C_VERSION 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

add_executable(${PROJECT_NAME} 
    src/main.c
    src/window.c
)

find_package(Vulkan REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES} ${GLFW_LIBRARIES})
