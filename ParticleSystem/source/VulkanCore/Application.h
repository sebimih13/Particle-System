#pragma once

// TODO: Forward Declarations
#include "Window.h"
#include "GPUDevice.h"
#include "Renderer.h"

namespace VulkanCore {

    struct ApplicationConfiguration
    {
        const WindowConfiguration windowConfig;

        // TODO: add

        ApplicationConfiguration(const WindowConfiguration& windowConfig);
    };

    // TODO: de facut clasa virtuala
    class Application
    {
    public:
        // Constructors
        Application(const ApplicationConfiguration& config);

        // Destructor
        virtual ~Application();

        // Not copyable
        Application(const Application&) = delete;
        Application& operator = (const Application&) = delete;

        // Not moveable
        Application(Application&&) = delete;
        Application& operator = (Application&&) = delete;

        // TODO: move
        void Run();

    private:
        Window window;
        GPUDevice device;
        SwapChain swapChain;
        Pipeline graphicsPipeline;
        Renderer renderer;

        bool bIsRunning; // TODO: de facut o functie Close()
    };

} // namespace VulkanCore
