cmake_minimum_required(VERSION 3.20)


project(Vulkan VERSION )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



set(SHADERC_SKIP_TESTS ON)
set(SHADERC_SKIP_EXAMPLES ON)
set(SHADERC_SPIRV_TOOLS_DIR ${CMAKE_CURRENT_LIST_DIR}/Vendor/spirv-tools/ )
set(SHADERC_SPIRV_HEADERS_DIR ${CMAKE_CURRENT_LIST_DIR}/Vendor/spirv-data/spirv-headers)
set(SPIRV-Headers_SOURCE_DIR ${SHADERC_SPIRV_HEADERS_DIR})

add_subdirectory(Vendor/spirv-tools)
add_subdirectory(Vendor/glslang)
add_subdirectory(Vendor/shaderc)


set(BUILD_SHARED_LIBS ON)
set(ENABLE_CTEST OFF "Enables testing")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(Vendor/GLFW)
add_subdirectory(Vendor/spdlog)
add_subdirectory(Vendor/Vulkan)

set(GAME_NAME Snake)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_LIST_DIR}/bin/)


set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${GAME_NAME}/shaders)
file(GLOB SHADERS ${SHADER_DIR}/*.vert ${SHADER_DIR}/*.frag ${SHADER_DIR}/*.comp ${SHADER_DIR}/*.geom ${SHADER_DIR}/*.tesc ${SHADER_DIR}/*.tese ${SHADER_DIR}/*.mesh ${SHADER_DIR}/*.task ${SHADER_DIR}/*.rgen ${SHADER_DIR}/*.rchit ${SHADER_DIR}/*.rmiss)

set(Vulkan_GLSLC_EXECUTABLE $<TARGET_FILE:glslc_exe>)

foreach(SHADER IN LISTS SHADERS)
    get_filename_component(FILENAME ${SHADER} NAME)
    add_custom_command(OUTPUT ${SHADER_DIR}/${FILENAME}.spv
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${SHADER} -o ${SHADER_DIR}/${FILENAME}.spv
        DEPENDS ${SHADER}
        COMMENT "Compiling ${FILENAME}")
list(APPEND SPV_SHADERS ${SHADER_DIR}/${FILENAME}.spv)
endForeach()

add_definitions()

add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})
aux_source_directory(${GAME_NAME} GAME_SOURCE)
add_executable(${GAME_NAME} ${GAME_SOURCE} ${SHADERS})


foreach(SHADER IN LISTS SPV_SHADERS)
    add_custom_command(
    TARGET ${GAME_NAME} 
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${GAME_NAME}>/Shaders/
    COMMAND ${CMAKE_COMMAND} -E copy ${SHADER} $<TARGET_FILE_DIR:${GAME_NAME}>/Shaders/
    COMMAND ${CMAKE_COMMAND} -E copy_directory $<TARGET_FILE_DIR:${GAME_NAME}> ${CMAKE_CURRENT_LIST_DIR}
#   COMMAND ${CMAKE_COMMAND} -E rm -r -f ${CMAKE_CURRENT_LIST_DIR}/build/
#   COMMAND ${CMAKE_COMMAND} -E rm -r -f ${CMAKE_CURRENT_LIST_DIR}/Snake/
#   COMMAND ${CMAKE_COMMAND} -E rm -r -f  ${CMAKE_CURRENT_LIST_DIR}/Vendor/
#   COMMAND ${CMAKE_COMMAND} -E rm -r -f  ${CMAKE_CURRENT_LIST_DIR}/.gitignore
#   COMMAND ${CMAKE_COMMAND} -E rm -r -f  ${CMAKE_CURRENT_LIST_DIR}/.gitmodules
#   COMMAND ${CMAKE_COMMAND} -E rm -r -f ${CMAKE_CURRENT_LIST_DIR}/README.md
#   COMMAND ${CMAKE_COMMAND} -E rm -r -f ${CMAKE_CURRENT_LIST_DIR}/buildWindows.bat
#   COMMAND ${CMAKE_COMMAND} -E rm -r -f ${CMAKE_CURRENT_LIST_DIR}/bin
    )
endforeach()


add_compile_definitions(
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:DIST>
    $<$<CONFIG:RelWithDebInfo>:RELEASE>
    $<$<CONFIG:MinSizeRel>:RelWithDebInfo>
)

target_include_directories(${GAME_NAME} PUBLIC Vendor/Vulkan/include Vendor/GLFW/Include Vendor/vkfw/Include Vendor/spdlog/Include)
target_link_libraries(${GAME_NAME} glfw spdlog)

add_custom_command(
    TARGET ${GAME_NAME} 
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:glfw> ${EXECUTABLE_OUTPUT_PATH}/$<CONFIG>/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:spdlog> ${EXECUTABLE_OUTPUT_PATH}/$<CONFIG>/
)

add_dependencies(${GAME_NAME} shaders)
