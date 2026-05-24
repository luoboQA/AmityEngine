cmake_minimum_required(VERSION 3.20)

include(FetchContent)

# Declare GoogleTest dependency
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

# Windows MSVC setting to avoid compiler warnings / CRT conflicts
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Make GoogleTest targets available
FetchContent_MakeAvailable(googletest)

# Define the unit test executable
add_executable(AmityTests
    TestEntityComponent.cpp
    TestScene.cpp
    TestPanTilt.cpp
    TestUI.cpp
    TestUserInput.cpp
    TestPhysics.cpp
    TestEvent.cpp
    TestResourceManager.cpp
    TestScripting.cpp
)

# Link tests to the Core engine and GoogleTest libraries
target_link_libraries(AmityTests PRIVATE
    AmityCore
    gtest_main
    gtest
)

# Register tests with CTest automatically
include(GoogleTest)
gtest_discover_tests(AmityTests)
