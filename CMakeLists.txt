cmake_minimum_required(VERSION 3.19)
project(cvulkan)

set(CMAKE_CXX_STANDARD 20)
file(GLOB_RECURSE src CONFIGURE_DEPENDS "include/*.h" "include/*.hpp" "src/*.cpp")


add_executable(cvulkan ${src})
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

