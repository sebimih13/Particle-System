#include "VulkanCore/Application.h"

#include <iostream>

int main(int argc, char* argv[])
{
    std::cout << "Hello World!\n\n";

    try
    {
        VulkanCore::WindowConfiguration WindowConfig(1200, 800, "Vulkan");
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
