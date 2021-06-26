cmake_minimum_required(VERSION 3.20)


project(Vulkan VERSION 0.0.1)

set(CMAKE_CXX_STANDARD_REQUIRED 17)

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/Snake SNAKE_SOURCES)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_LIST_DIR}/bin/Debug/Snake)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(Vendor/GLFW)
find_package(Vulkan REQUIRED FATAL_ERROR)



add_executable(Snake ${SNAKE_SOURCES})

target_link_libraries(Snake Vulkan::Vulkan)
target_include_directories(Snake PUBLIC Vulkan::Headers)

target_link_libraries(Snake glfw)
