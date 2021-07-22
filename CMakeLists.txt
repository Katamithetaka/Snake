
cmake_minimum_required(VERSION 3.20.2)
project(Vulkan VERSION 1.0.0)

set(CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED ON)


if("${CMAKE_BUILD_TYPE}" STREQUAL "") 
    set(CMAKE_BUILD_TYPE Debug)
endif()


set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)

add_compile_definitions(
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:MinSizeRel>:DIST>
    $<$<CONFIG:RelWithDebInfo>:RELEASE>
    $<$<CONFIG:Release>:DIST>
)

set(BUILD_SHARED_LIBS ON)

set(GAME_NAME Snake)

aux_source_directory(${GAME_NAME}/ ${GAME_NAME}_SRC_LIST)
aux_source_directory(${GAME_NAME}/Mountain MOUNTAIN_SRC_LIST)
add_executable(${GAME_NAME} ${${GAME_NAME}_SRC_LIST} ${MOUNTAIN_SRC_LIST})

set_property(TARGET ${GAME_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${GAME_NAME} PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin/$<CONFIG>)
set_property(TARGET ${GAME_NAME} PROPERTY PDB_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin-int/)

set(SHADER_DIR ${CMAKE_CURRENT_LIST_DIR}/${GAME_NAME}/Shaders)

message(${CMAKE_BUILD_TYPE})
if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")

    find_package(Vulkan REQUIRED)

    if(${Vulkan_Found})
        set(GLSLC ${Vulkan_GLSLC_EXECUTABLE})
    endif()


    file(GLOB SHADERS ${SHADER_DIR}/*.vert ${SHADER_DIR}/*.frag ${SHADER_DIR}/*.comp ${SHADER_DIR}/*.geom ${SHADER_DIR}/*.tesc ${SHADER_DIR}/*.tese ${SHADER_DIR}/*.mesh ${SHADER_DIR}/*.task ${SHADER_DIR}/*.rgen ${SHADER_DIR}/*.rchit ${SHADER_DIR}/*.rmiss)

    add_custom_command(TARGET ${GAME_NAME}
        POST_BUILD 
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_DIR}/Compiled
    )

    foreach(SHADER IN LISTS SHADERS)
        get_filename_component(FILENAME ${SHADER} NAME)
        add_custom_command(TARGET ${GAME_NAME} POST_BUILD
            COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${SHADER} -o ${SHADER_DIR}/Compiled/${FILENAME}.spv
            DEPENDS ${SHADER}
            COMMENT "Compiling ${FILENAME}"
        )
        list(APPEND SPV_SHADERS ${SHADER_DIR}/Compiled/${FILENAME}.spv)
    endforeach()

    add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})

    add_dependencies(${GAME_NAME} shaders)
    
endif()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Vendor/GLFW)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Vendor/spdlog)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Vendor/glm)
target_include_directories(${GAME_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/Vendor/GLFW/include ${CMAKE_CURRENT_LIST_DIR}/Vendor/spdlog/include ${CMAKE_CURRENT_LIST_DIR}/Vendor/vkfw/include ${CMAKE_CURRENT_LIST_DIR}/Vendor/Vulkan/include ${CMAKE_CURRENT_LIST_DIR}/Vendor/glm/)
if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/Vendor/Vulkan/include/vulkan/vulkan.hpp)
    message(FATAL "Couldn't find vulkan.hpp")
endif()


target_link_libraries(${GAME_NAME} spdlog glfw ${CMAKE_DL_LIBS} glm)


add_custom_command(TARGET ${GAME_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${GAME_NAME}>/Shaders/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:glfw> $<TARGET_FILE_DIR:${GAME_NAME}>/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:spdlog> $<TARGET_FILE_DIR:${GAME_NAME}>/
    COMMAND ${CMAKE_COMMAND} -E copy_directory  ${SHADER_DIR}/Compiled $<TARGET_FILE_DIR:${GAME_NAME}>/Shaders/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/default${GAME_NAME}Config.txt $<TARGET_FILE_DIR:${GAME_NAME}>/defaultConfig.txt
    COMMAND cd $<TARGET_FILE_DIR:${GAME_NAME}>/
)


