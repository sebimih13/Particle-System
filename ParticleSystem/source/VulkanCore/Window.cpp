#include "Window.h"

#include <stdexcept>

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

    void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
    {
        if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create window surface");
        }
    }

    // TODO: de revazut
    int Window::GetWidth() const
    {
        int width;
        glfwGetFramebufferSize(window, &width, nullptr);
        
        return width;
    }

    // TODO: de revazut
    int Window::GetHeight() const
    {
        int height;
        glfwGetFramebufferSize(window, nullptr, &height);

        return height;
    }

} // namespace VulkanCore
