cmake_minimum_required(VERSION 3.0.0)
project(cpp-is-fun VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(cpp-is-fun main.cpp renderer.cpp matrices.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

include_directories(${CMAKE_SOURCE_DIR}/Eigen)

file(COPY ${CMAKE_SOURCE_DIR}/res DESTINATION ${CMAKE_BINARY_DIR})

find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(cpp-is-fun
    PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)