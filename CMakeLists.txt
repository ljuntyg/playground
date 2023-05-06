cmake_minimum_required(VERSION 3.0.0)
project(reground VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(reground main.cpp renderer.cpp obj_loader.cpp structs.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

file(COPY ${CMAKE_SOURCE_DIR}/res DESTINATION ${CMAKE_BINARY_DIR})

find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(reground
    PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

find_package(OpenGL REQUIRED)
target_link_libraries(reground PRIVATE OpenGL::GL)

find_package(glm CONFIG REQUIRED)
target_link_libraries(reground PRIVATE glm)

find_package(GLEW REQUIRED)
target_link_libraries(reground PRIVATE GLEW::GLEW)