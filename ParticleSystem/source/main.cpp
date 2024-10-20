#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>

static constexpr int WIDTH = 800;
static constexpr int HEIGHT = 600;

int main()
{
    std::cout << "Hello World!" << std::endl;

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

    while (!glfwWindowShouldClose(window))
    {
        // loop
    }

    return EXIT_SUCCESS;
}
