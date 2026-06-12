cmake_minimum_required(VERSION 3.20)
project(AmityEngine LANGUAGES CXX C)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(AMITY_ENABLE_LUA "Enable LuaScriptService" OFF)

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

# Lua Dependency Configuration (Statically compiled from official source)
if(AMITY_ENABLE_LUA)
    include(cmake/Lua.cmake)
endif()

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
    src/core/user_input/UserInputService.cpp
    src/core/WaterRenderable.cpp
)


# Core Include Directories
target_include_directories(AmityCore PUBLIC src/core)
target_include_directories(AmityCore PUBLIC ${PROJECT_SOURCE_DIR}/src/third_party/stb)

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

# ============================================================================
# 复制资源文件（只执行一次，所有游戏共享）
# ============================================================================
add_custom_target(CopyResources ALL
    # 创建目录
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/src/shaders"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/src/games/AmityLabyrinth/shaders"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/assets"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/LowPolyAssets"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/soundAssets"
    
    # 删除多余的 shaders 目录（如果存在）
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${CMAKE_BINARY_DIR}/shaders"
    
    # 复制全局 shaders 到 src/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/src/shaders"
        "${CMAKE_BINARY_DIR}/src/shaders"
    
    # 复制 AmityLabyrinth 的专属 shaders 到 src/games/AmityLabyrinth/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/src/games/AmityLabyrinth/shaders"
        "${CMAKE_BINARY_DIR}/src/games/AmityLabyrinth/shaders"
    
    # 复制其他资源
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/src/assets"
        "${CMAKE_BINARY_DIR}/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/src/LowPolyAssets"
        "${CMAKE_BINARY_DIR}/LowPolyAssets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/src/soundAssets"
        "${CMAKE_BINARY_DIR}/soundAssets"
    
    COMMENT "Copying resources once to build directory"
)

# ============================================================================
# 游戏可执行文件
# ============================================================================

# Main Game Executable Target
add_executable(Amity
    src/app/main.cpp
    src/app/TestGame.cpp
)
target_link_libraries(Amity PRIVATE AmityCore)
add_dependencies(Amity CopyResources)

if(AMITY_ENABLE_LUA)
    target_sources(AmityCore PRIVATE src/core/scripting/LuaScriptService.cpp)
    target_include_directories(AmityCore PUBLIC "${lua_SOURCE_DIR}/src")
    target_link_libraries(AmityCore PUBLIC lua)
    target_compile_definitions(AmityCore PUBLIC AMITY_HAS_LUA=1)
    add_custom_command(
        TARGET Amity POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                "${CMAKE_CURRENT_SOURCE_DIR}/src/app/script.lua"
                "$<TARGET_FILE_DIR:Amity>/script.lua"
        COMMENT "copy lua script to build dir"
    )
endif()

# Voxel Minecraft Clone Executable Target (AmityCraft)
add_executable(AmityCraft
    src/games/AmityCraft/main.cpp
    src/games/AmityCraft/CraftMinerGame.cpp
    src/games/AmityCraft/PlayerController.cpp
    src/games/AmityCraft/VoxelWorld.cpp
)
target_link_libraries(AmityCraft PRIVATE AmityCore)
target_include_directories(AmityCraft PRIVATE src/games/AmityCraft)
add_dependencies(AmityCraft CopyResources)

# Pan/Tilt Camera Showcase Target (PanTiltShowcase)
add_executable(PanTiltShowcase
    src/games/PanTiltShowcase/main.cpp
    src/games/PanTiltShowcase/PanTiltShowcaseGame.cpp
)
target_link_libraries(PanTiltShowcase PRIVATE AmityCore)
target_include_directories(PanTiltShowcase PRIVATE src/games/PanTiltShowcase)
add_dependencies(PanTiltShowcase CopyResources)

# Canyon Siege Game Target (CanyonSiege)
add_executable(CanyonSiege
    src/games/CanyonSiege/main.cpp
    src/games/CanyonSiege/CanyonSiegeGame.cpp
)
target_link_libraries(CanyonSiege PRIVATE AmityCore)
target_include_directories(CanyonSiege PRIVATE src/games/CanyonSiege)
target_include_directories(CanyonSiege PRIVATE src/third_party/stb)
add_dependencies(CanyonSiege CopyResources)

# Shadows of Amity Labyrinth horror game target (AmityLabyrinth)
add_executable(AmityLabyrinth
    src/games/AmityLabyrinth/main.cpp
    src/games/AmityLabyrinth/LabyrinthGame.cpp
)
target_link_libraries(AmityLabyrinth PRIVATE AmityCore)
target_include_directories(AmityLabyrinth PRIVATE src/games/AmityLabyrinth)
add_dependencies(AmityLabyrinth CopyResources)

# Stormy Sails Sailing Game target (StormySails)
add_executable(StormySails
    src/games/StormySails/main.cpp
    src/games/StormySails/StormySailsGame.cpp
)
target_link_libraries(StormySails PRIVATE AmityCore)
target_include_directories(StormySails PRIVATE src/games/StormySails)
target_include_directories(StormySails PRIVATE src/third_party/stb)
add_dependencies(StormySails CopyResources)

# build Private Games
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/games/PrivateGames/CMakeLists.txt")
    add_subdirectory(src/games/PrivateGames)
endif()

# Testing Suite Integration
# enable_testing()
# add_subdirectory(tests)