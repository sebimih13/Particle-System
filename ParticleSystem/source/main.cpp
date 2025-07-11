// Project Global defines
// #define GLFW_INCLUDE_VULKAN
// #define GLFW_INCLUDE_NONE
// #define GLM_FORCE_RADIANS
// #define GLM_FORCE_DEPTH_ZERO_TO_ONE
// #define GLM_ENABLE_EXPERIMENTAL

#include "VulkanCore/Application.h"

#include <iostream>

int main(int argc, char* argv[])
{
    std::cout << "Hello World!\n\n";

    try
    {
        VulkanCore::WindowConfiguration WindowConfig(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, "Particle System");
        VulkanCore::ApplicationConfiguration AppConfig(WindowConfig);

        VulkanCore::Application App(AppConfig);
        App.Run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
