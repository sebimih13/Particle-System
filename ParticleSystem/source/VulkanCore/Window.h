#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <string>

namespace VulkanCore {
    
    struct WindowConfiguration
    {
        const uint32_t Width;
        const uint32_t Height;
        const std::string Title;

        WindowConfiguration(const uint32_t& Width, const uint32_t& Height, const std::string& Title);
    };

    class Window
    {
    public:
        // Constructors
        Window(const WindowConfiguration& config);

        // Destructor
        ~Window();

        // Not copyable
		Window(const Window&) = delete;
		Window& operator = (const Window&) = delete;

        // Not moveable
        Window(Window&&) = delete;
        Window& operator = (Window&&) = delete;

        void Update();

        // TODO: se poate mai frumos
        inline bool ShouldClose() { return glfwWindowShouldClose(MainWindow); }

    private:
        GLFWwindow* MainWindow;
    };

} // namespace VulkanCore
