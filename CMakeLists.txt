cmake_minimum_required(VERSION 3.0.0)
project(cpp-is-fun VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(cpp-is-fun main.cpp Renderer.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(cpp-is-fun
    PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)