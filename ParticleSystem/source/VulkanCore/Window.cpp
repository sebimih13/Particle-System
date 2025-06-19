#include "Window.h"

#include <stdexcept>

namespace VulkanCore {

    WindowConfiguration::WindowConfiguration(const uint32_t& width, const uint32_t& height, const std::string& title)
        : width(width), height(height), title(title)
    {

    }

    Window::Window(const WindowConfiguration& config)
        : framebufferResized(false)
    {
        if (!glfwInit())
        {
            throw std::runtime_error("Could not initalize GLFW!");
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(config.width, config.height, config.title.c_str(), nullptr, nullptr);

        glfwSetWindowPos(window, 350, 150);

        // Set GLFW callbacks
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);
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

    void Window::CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
    {
        if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create window surface");
        }
    }

    void Window::BlockWindow()
    {
        glfwSetWindowSize(window, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
        glfwSetWindowPos(window, 350, 150);
        glfwSetWindowAttrib(window, GLFW_RESIZABLE, GLFW_FALSE);
        glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
    }

    void Window::UnblockWindow()
    {
        glfwSetWindowAttrib(window, GLFW_RESIZABLE, GLFW_TRUE);
        glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_TRUE);
    }

    int Window::GetWidth() const
    {
        int width;
        glfwGetFramebufferSize(window, &width, nullptr);
        
        return width;
    }

    int Window::GetHeight() const
    {
        int height;
        glfwGetFramebufferSize(window, nullptr, &height);

        return height;
    }

    void Window::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        Window* windowPtr = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        windowPtr->framebufferResized = true;
    }

} // namespace VulkanCore
