cmake_minimum_required(VERSION 3.20)
project(AmityEngine LANGUAGES CXX C)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(Amity
    src/app/main.cpp
    src/app/TestGame.cpp
    src/core/Application.cpp
    src/core/Shader.cpp
    src/core/ModelRenderable.cpp
    src/core/TerrainRenderable.cpp
    src/core/Scene.cpp
    src/core/Entity.cpp
    src/core/Sound.cpp
)

target_include_directories(Amity PRIVATE src/core)
target_include_directories(Amity PRIVATE ${PROJECT_SOURCE_DIR}/src/third_party/stb)

# opengl packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(assimp REQUIRED)
find_package(OpenAL REQUIRED)
find_package(LibSndFile REQUIRED)

#glad
add_library(glad STATIC src/third_party/glad/glad.c)

# copy shaders
add_custom_command(
    TARGET Amity POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/src/shaders"
            "$<TARGET_FILE_DIR:Amity>/shaders"
    COMMENT "copy shaders to build dir"
)

target_include_directories(
    glad PUBLIC src/third_party/glad/include
)

target_link_libraries(Amity PRIVATE
    glfw
    OpenGL::GL
    glad
    glm::glm
    assimp::assimp
    # OpenAL::OpenAL
    ${LIBSNDFILE_LIBRARIES}
)

# openal libs (sometimes doesnt use OpenAL::OpenAL)
if(TARGET OpenAL::OpenAL)
    # Modern way (e.g., Windows with vcpkg)
    # The target handles includes and linking
    message(STATUS "Found OpenAL via modern target 'OpenAL::OpenAL'")
    target_link_libraries(Amity PRIVATE OpenAL::OpenAL)
else()
    # Legacy way (e.g., default on Linux)
    # Manually add includes and link the library variable
    message(STATUS "Found OpenAL via legacy variables")
    target_include_directories(Amity PRIVATE ${OPENAL_INCLUDE_DIR})
    target_link_libraries(Amity PRIVATE ${OPENAL_LIBRARY})
endif()
