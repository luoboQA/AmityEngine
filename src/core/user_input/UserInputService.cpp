#include "UserInputService.hpp"

namespace Core {

UserInputService::UserInputService(GLFWwindow* window) :
    m_window(window)
{

}

bool UserInputService::isKeyDown(KeyCode keyCode) const
{
    return glfwGetKey(m_window, static_cast<int>(keyCode)) == GLFW_PRESS;
}

}