#include "Window.h"

namespace VulkanCore {

    WindowConfiguration::WindowConfiguration(const uint32_t& Width, const uint32_t& Height, const std::string& Title)
        : Width(Width), Height(Height), Title(Title)
    {

    }

    Window::Window(const WindowConfiguration& config)
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        MainWindow = glfwCreateWindow(config.Width, config.Height, config.Title.c_str(), nullptr, nullptr);

        // Set GLFW callbacks
        // TODO: glfwSetWindowSizeCallback
        // TODO: glfwSetWindowCloseCallback
        // TODO: glfwSetKeyCallback
        // TODO: glfwSetCharCallback
        // TODO: glfwSetMouseButtonCallback
        // TODO: glfwSetScrollCallback
        // TODO: glfwSetCursorPosCallback
    }

    Window::~Window()
    {
        glfwDestroyWindow(MainWindow);
        glfwTerminate();
    }

    void Window::Update()
    {
        glfwPollEvents();
    }

} // namespace VulkanCore
