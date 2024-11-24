#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <string>

namespace VulkanCore {
    
    struct WindowConfiguration final
    {
        const uint32_t width;
        const uint32_t height;
        const std::string title;

        WindowConfiguration(const uint32_t& width, const uint32_t& height, const std::string& title);
    };

    class Window final
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
        inline bool ShouldClose() const { return glfwWindowShouldClose(window); }

        void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

        inline void ResetWindowResizedFlag() { framebufferResized = false; }

        // Getters
        inline GLFWwindow* const GetGLFWWindow() const { return window; }
        int GetWidth() const;
        int GetHeight() const;
        inline bool GetWasWindowResized() const { return framebufferResized; }

    private:
        GLFWwindow* window;

        bool framebufferResized;

        // Callbacks
        static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
    };

} // namespace VulkanCore
