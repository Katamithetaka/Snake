
cmake_minimum_required(VERSION 3.20.2)
project(Vulkan VERSION 1.0.0)

if(CMAKE_BUILD_TYPE STREQUAL "") 
    set(CMAKE_BUILD_TYPE "Debug")
endif()


add_compile_definitions(
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:MinSizeRel>:DIST>
    $<$<CONFIG:RelWithDebInfo>:RELEASE>
    $<$<CONFIG:Release>:RELEASE>
)

