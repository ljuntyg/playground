cmake_minimum_required(VERSION 3.0.0)
project(playground VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)
enable_testing()

add_executable(playground main.cpp obj_loader.cpp renderer.cpp text.cpp gui.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

file(COPY ${CMAKE_SOURCE_DIR}/res DESTINATION ${CMAKE_BINARY_DIR})

find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(playground
    PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

find_package(OpenGL REQUIRED)
target_link_libraries(playground PRIVATE OpenGL::GL)

find_package(glm CONFIG REQUIRED)
target_link_libraries(playground PRIVATE glm::glm)

find_package(glad CONFIG REQUIRED)
target_link_libraries(playground PRIVATE glad::glad)