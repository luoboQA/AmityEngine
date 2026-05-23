#pragma once

#include "KeyCode.hpp"
#include <GLFW/glfw3.h>

namespace Core {

class UserInputService
{
public:
    UserInputService(GLFWwindow* window) : m_window(window) {}

    bool isKeyDown(KeyCode keyCode) const;

private:
    GLFWwindow* m_window;

};

}