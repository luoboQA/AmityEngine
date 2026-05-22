#include "Application.hpp"

namespace Core {

Application::Application(int width, int height) : WIDTH(width), HEIGHT(height), m_lastFrameTime(glfwGetTime())
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
    }
    
    m_startTime = glfwGetTime();

    m_window = glfwCreateWindow(WIDTH, HEIGHT, m_appName.c_str(), nullptr, nullptr);
    if (!m_window)
    {
        std::cerr << "Couldn't create window :(" << std::endl;
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(m_window);


    // openAL SETUP
    device = alcOpenDevice(nullptr);
    if (!device) throw std::runtime_error("No audio device");
    context = alcCreateContext(device, nullptr);
    if (!alcMakeContextCurrent(context)) throw std::runtime_error("Can't make device context current");


    // glad init
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
    }

    glViewport(0, 0, WIDTH, HEIGHT);
    m_scene.setScreenSize(WIDTH, HEIGHT);
    m_scene.setupFramebuffer();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // resize callback
    glfwSetWindowUserPointer(m_window, this);
    glfwSetWindowSizeCallback(m_window, Application::onResizeCallback);

    setupProjectionMatrix();

}

Application::~Application()
{
    // cleanup
    glfwDestroyWindow(m_window);
    glfwTerminate();

    // openAL cleanup
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    alcCloseDevice(device);
}


// RESIZING
void Application::onResizeCallback(GLFWwindow* window, int width, int height)
{
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (app)
        app->onResize(window, width, height);
}
void Application::onResize(GLFWwindow* window, int width, int height)
{
    WIDTH = width;
    HEIGHT = height;
    glViewport(0, 0, WIDTH, HEIGHT);
    setupProjectionMatrix();
    m_scene.setScreenSize(WIDTH, HEIGHT);
}

void Application::setupProjectionMatrix()
{
    float aspect = (HEIGHT > 0) ? (static_cast<float>(WIDTH) / HEIGHT) : 1.333f;

    // Synchronize the active camera aspect ratio if it exists!
    if (auto cameraEntity = m_scene.getCameraEntity())
    {
        if (auto cameraComp = cameraEntity->getComponent<CameraComponent>())
        {
            cameraComp->setAspect(aspect);
        }
    }
}

int Application::run()
{
    while (!glfwWindowShouldClose(m_window))
    {
        // get deltatime
        double t = glfwGetTime();
        double dt = t - m_lastFrameTime;
        m_lastFrameTime = t;
        m_runTime = t - m_startTime;

        // update (maybe move this to separate timer later)
        m_scene.update(dt);
        update(dt);

        // RENDER
        glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_scene.render(dt);

        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }

    return 0;
}



} // Core namespace