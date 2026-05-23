#include <gtest/gtest.h>
#include "user_input/KeyCode.hpp"
#include <GLFW/glfw3.h>

using namespace Core;

// Test: Verify alphabet KeyCode mappings (A-Z)
TEST(UserInputServiceTest, AlphabetMapping)
{
    EXPECT_EQ(static_cast<int>(KeyCode::A), GLFW_KEY_A);
    EXPECT_EQ(static_cast<int>(KeyCode::B), GLFW_KEY_B);
    EXPECT_EQ(static_cast<int>(KeyCode::M), GLFW_KEY_M);
    EXPECT_EQ(static_cast<int>(KeyCode::Z), GLFW_KEY_Z);
}

// Test: Verify digit KeyCode mappings (Zero-Nine)
TEST(UserInputServiceTest, DigitMapping)
{
    EXPECT_EQ(static_cast<int>(KeyCode::Zero), GLFW_KEY_0);
    EXPECT_EQ(static_cast<int>(KeyCode::Five), GLFW_KEY_5);
    EXPECT_EQ(static_cast<int>(KeyCode::Nine), GLFW_KEY_9);
}

// Test: Verify navigation and arrow KeyCode mappings
TEST(UserInputServiceTest, NavigationMapping)
{
    EXPECT_EQ(static_cast<int>(KeyCode::Up), GLFW_KEY_UP);
    EXPECT_EQ(static_cast<int>(KeyCode::Down), GLFW_KEY_DOWN);
    EXPECT_EQ(static_cast<int>(KeyCode::Left), GLFW_KEY_LEFT);
    EXPECT_EQ(static_cast<int>(KeyCode::Right), GLFW_KEY_RIGHT);
    EXPECT_EQ(static_cast<int>(KeyCode::Escape), GLFW_KEY_ESCAPE);
    EXPECT_EQ(static_cast<int>(KeyCode::Space), GLFW_KEY_SPACE);
}

// Test: Verify keypad numbers and symbols mapping
TEST(UserInputServiceTest, KeypadMapping)
{
    EXPECT_EQ(static_cast<int>(KeyCode::KeypadZero), GLFW_KEY_KP_0);
    EXPECT_EQ(static_cast<int>(KeyCode::KeypadNine), GLFW_KEY_KP_9);
    EXPECT_EQ(static_cast<int>(KeyCode::KeypadEnter), GLFW_KEY_KP_ENTER);
    EXPECT_EQ(static_cast<int>(KeyCode::KeypadPlus), GLFW_KEY_KP_ADD);
}

// Test: Verify function key mappings (F1-F15)
TEST(UserInputServiceTest, FunctionKeysMapping)
{
    EXPECT_EQ(static_cast<int>(KeyCode::F1), GLFW_KEY_F1);
    EXPECT_EQ(static_cast<int>(KeyCode::F12), GLFW_KEY_F12);
    EXPECT_EQ(static_cast<int>(KeyCode::F15), GLFW_KEY_F15);
}

// Test: Verify modifier key mappings
TEST(UserInputServiceTest, ModifiersMapping)
{
    EXPECT_EQ(static_cast<int>(KeyCode::LeftShift), GLFW_KEY_LEFT_SHIFT);
    EXPECT_EQ(static_cast<int>(KeyCode::RightShift), GLFW_KEY_RIGHT_SHIFT);
    EXPECT_EQ(static_cast<int>(KeyCode::LeftControl), GLFW_KEY_LEFT_CONTROL);
    EXPECT_EQ(static_cast<int>(KeyCode::LeftAlt), GLFW_KEY_LEFT_ALT);
}

// Test: Unknown keycode should match GLFW_KEY_UNKNOWN (-1)
TEST(UserInputServiceTest, UnknownMapping)
{
    EXPECT_EQ(static_cast<int>(KeyCode::Unknown), GLFW_KEY_UNKNOWN);
}
