#include "Window.h"

namespace VulkanCore {

    WindowConfiguration::WindowConfiguration(const uint32_t& width, const uint32_t& height, const std::string& title)
        : width(width), height(height), title(title)
    {

    }

    Window::Window(const WindowConfiguration& config)
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(config.width, config.height, config.title.c_str(), nullptr, nullptr);

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
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void Window::Update()
    {
        glfwPollEvents();
    }

} // namespace VulkanCore
