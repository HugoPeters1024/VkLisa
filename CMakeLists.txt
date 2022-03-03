cmake_minimum_required(VERSION 3.19)
project(cvulkan)

set(CMAKE_CXX_STANDARD 20)
file(GLOB_RECURSE src CONFIGURE_DEPENDS "include/*.h" "include/*.hpp" "src/*.cpp")

macro(shader)
    add_custom_command(
            OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/shaders_bin/${ARGV0}.spv
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/${ARGV0}
            COMMAND /usr/bin/glslc
            ARGS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/${ARGV0} -o ${CMAKE_CURRENT_SOURCE_DIR}/shaders_bin/${ARGV0}.spv -O --target-env=vulkan1.2
            COMMENT building shaders
            VERBATIM)
    SET(shader_src ${shader_src} ${CMAKE_CURRENT_SOURCE_DIR}/shaders_bin/${ARGV0}.spv)
endmacro()

shader("grid.vert")
shader("grid.frag")
shader("quad.vert")
shader("quad.frag")
shader("reduce.comp")
shader("evolve.comp")
shader("lottery.comp")


add_executable(cvulkan ${src} ${shader_src})
target_precompile_headers(cvulkan PRIVATE include/precomp.h)
target_include_directories(cvulkan PRIVATE include)

add_subdirectory(${CMAKE_SOURCE_DIR}/external/vks)
link_directories(${CMAKE_SOURCE_DIR}/external/vks)
include_directories(cvulkan PUBLIC ${CMAKE_SOURCE_DIR}/external/vks/)
target_link_libraries(cvulkan vks)

add_subdirectory(${CMAKE_SOURCE_DIR}/external/glfw)
link_directories(${CMAKE_SOURCE_DIR}/external/glfw)
include_directories(cvulkan PUBLIC ${CMAKE_SOURCE_DIR}/external/glfw/include/)
target_link_libraries(cvulkan glfw)

add_subdirectory(${CMAKE_SOURCE_DIR}/external/glm)
link_directories(${CMAKE_SOURCE_DIR}/external/glm)
include_directories(cvulkan PUBLIC ${CMAKE_SOURCE_DIR}/external/glm/glm/)
target_link_libraries(cvulkan glm)

add_subdirectory(${CMAKE_SOURCE_DIR}/external/spdlog)
link_directories(${CMAKE_SOURCE_DIR}/external/spdlog)
include_directories(cvulkan PRIVATE ${CMAKE_SOURCE_DIR}/external/spdlog/include/)
target_link_libraries(cvulkan spdlog)

find_package(Vulkan REQUIRED)
target_link_libraries(cvulkan Vulkan::Vulkan)

