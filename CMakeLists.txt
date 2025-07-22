cmake_minimum_required(VERSION 3.11)
set(PROJECT_NAME project_base)
project(${PROJECT_NAME})

function(watch)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${ARGV})
endfunction()

set(CMAKE_POLICY_DEFAULT_CMP0012 NEW)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")


file(GLOB SOURCES "src/*.cpp" "src/*.c" "src/main.cpp" "libs/stb_image.cpp")
message(STATUS "SOURCES found: ${SOURCES}")

list(APPEND SOURCES libs/stb_image.cpp)
set_source_files_properties(libs/stb_image.cpp PROPERTIES COMPILE_FLAGS "-Wno-shift-negative-value -Wno-implicit-fallthrough")


file(GLOB HEADERS "include/*.h" "include/*.hpp")
message(STATUS "HEADERS found: ${HEADERS}")

find_package(OpenGL REQUIRED)
find_package(GLFW3 REQUIRED)
find_package(ASSIMP REQUIRED)
find_package(glm REQUIRED)

add_subdirectory(libs/glad)
add_subdirectory(libs/imgui)

set(LIBS
    glfw
    glad
    OpenGL::GL
    X11 Xrandr Xinerama Xi Xxf86vm Xcursor
    dl pthread freetype
    ${ASSIMP_LIBRARIES}
    imgui
)

configure_file(configuration/root_directory.h.in configuration/root_directory.h)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        include/
        libs/
        ${CMAKE_BINARY_DIR}/configuration
)

target_compile_options(${PROJECT_NAME}
    PRIVATE
        -g -Wall -Wextra -Wno-unused-variable -Wno-unused-parameter -O3
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        ${OPENGL_DEFINITIONS}
)

target_link_libraries(${PROJECT_NAME} ${LIBS})

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}")

file(GLOB SHADERS "shaders/*.vs" "shaders/*.fs")
foreach(SHADER ${SHADERS})
    watch(${SHADER})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders
        COMMAND ${CMAKE_COMMAND} -E copy ${SHADER} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders)
endforeach()
