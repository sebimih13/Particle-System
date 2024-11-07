#include "VulkanCore/Application.h"

#include <iostream>

int main(int argc, char* argv[])
{
    std::cout << "Hello World!" << std::endl;

    try
    {
        VulkanCore::WindowConfiguration WindowConfig(600, 800, "Vulkan");
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
