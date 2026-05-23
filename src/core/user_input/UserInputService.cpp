#include "UserInputService.hpp"

namespace Core {

bool UserInputService::isKeyDown(KeyCode keyCode) const
{
    return glfwGetKey(m_window, static_cast<int>(keyCode)) == GLFW_PRESS;
}

}