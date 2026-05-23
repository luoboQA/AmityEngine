cmake_minimum_required(VERSION 3.20)
project(AmityEngine LANGUAGES CXX C)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable parallel compilation for MSVC to dramatically speed up Windows builds
if(MSVC)
    add_compile_options(/MP)
endif()

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
    src/core/FontManager.cpp
    src/core/UIRenderer.cpp
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


# Voxel Minecraft Clone Executable Target (AmityCraft)
add_executable(AmityCraft
    src/games/AmityCraft/main.cpp
    src/games/AmityCraft/CraftMinerGame.cpp
    src/games/AmityCraft/PlayerController.cpp
    src/games/AmityCraft/VoxelWorld.cpp
)
target_link_libraries(AmityCraft PRIVATE AmityCore)
target_include_directories(AmityCraft PRIVATE src/games/AmityCraft)

# Post-Build Custom Command to copy shaders next to the AmityCraft executable
add_custom_command(
    TARGET AmityCraft POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/src/shaders"
            "$<TARGET_FILE_DIR:AmityCraft>/shaders"
    COMMENT "copy shaders to AmityCraft build dir"
)

# Pan/Tilt Camera Showcase Target (PanTiltShowcase)
add_executable(PanTiltShowcase
    src/games/PanTiltShowcase/main.cpp
    src/games/PanTiltShowcase/PanTiltShowcaseGame.cpp
)
target_link_libraries(PanTiltShowcase PRIVATE AmityCore)
target_include_directories(PanTiltShowcase PRIVATE src/games/PanTiltShowcase)

# Post-Build Custom Command to copy shaders next to the PanTiltShowcase executable
add_custom_command(
    TARGET PanTiltShowcase POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/src/shaders"
            "$<TARGET_FILE_DIR:PanTiltShowcase>/shaders"
    COMMENT "copy shaders to PanTiltShowcase build dir"
)

# Canyon Siege Game Target (CanyonSiege)
add_executable(CanyonSiege
    src/games/CanyonSiege/main.cpp
    src/games/CanyonSiege/CanyonSiegeGame.cpp
)
target_link_libraries(CanyonSiege PRIVATE AmityCore)
target_include_directories(CanyonSiege PRIVATE src/games/CanyonSiege)
target_include_directories(CanyonSiege PRIVATE src/third_party/stb)

# Post-Build Custom Command to copy shaders next to the CanyonSiege executable
add_custom_command(
    TARGET CanyonSiege POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/src/shaders"
            "$<TARGET_FILE_DIR:CanyonSiege>/shaders"
    COMMENT "copy shaders to CanyonSiege build dir"
)

# build Private Games
if(EXISTS "${CMAKE_SOURCE_DIR}/src/games/PrivateGames/CMakeLists.txt")
    add_subdirectory(src/games/PrivateGames)
endif()

# Testing Suite Integration
enable_testing()
add_subdirectory(tests)
