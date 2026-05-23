#pragma once

#include "KeyCode.hpp"
#include "../Event.hpp"
#include <GLFW/glfw3.h>

namespace Core {

class UserInputService
{
public:
    UserInputService(GLFWwindow* window);

    bool isKeyDown(KeyCode keyCode) const;

    // EVENTS
    Event<KeyCode> InputBegan;
    Event<KeyCode> InputEnded;

private:
    GLFWwindow* m_window;

};

}