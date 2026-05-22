cmake_minimum_required(VERSION 3.20)
project(AmityEngine LANGUAGES CXX C)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find Package Dependencies (needed by core engine)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(assimp REQUIRED)
find_package(OpenAL REQUIRED)
find_package(LibSndFile REQUIRED)

# Glad OpenGL Loader Library
add_library(glad STATIC src/third_party/glad/glad.c)
target_include_directories(glad PUBLIC src/third_party/glad/include)

# Core Engine Static Library
add_library(AmityCore STATIC
    src/core/Application.cpp
    src/core/Shader.cpp
    src/core/ModelRenderable.cpp
    src/core/TerrainRenderable.cpp
    src/core/Scene.cpp
    src/core/Entity.cpp
    src/core/Sound.cpp
)

# Core Include Directories
target_include_directories(AmityCore PUBLIC src/core)
target_include_directories(AmityCore PRIVATE ${PROJECT_SOURCE_DIR}/src/third_party/stb)

# Core Link Libraries (Inherited transitively by downstream targets)
target_link_libraries(AmityCore PUBLIC
    glfw
    OpenGL::GL
    glad
    glm::glm
    assimp::assimp
    ${LIBSNDFILE_LIBRARIES}
)

# OpenAL Linking logic (Modern vs Legacy fallback)
if(TARGET OpenAL::OpenAL)
    target_link_libraries(AmityCore PUBLIC OpenAL::OpenAL)
else()
    target_include_directories(AmityCore PUBLIC ${OPENAL_INCLUDE_DIR})
    target_link_libraries(AmityCore PUBLIC ${OPENAL_LIBRARY})
endif()

# Main Game Executable Target
add_executable(Amity
    src/app/main.cpp
    src/app/TestGame.cpp
)
target_link_libraries(Amity PRIVATE AmityCore)

# Post-Build Custom Command to copy shaders next to the executable
add_custom_command(
    TARGET Amity POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/src/shaders"
            "$<TARGET_FILE_DIR:Amity>/shaders"
    COMMENT "copy shaders to build dir"
)

# Testing Suite Integration
enable_testing()
add_subdirectory(tests)
