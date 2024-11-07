#pragma once

// TODO: Forward Declarations
#include "Window.h"
#include "GPUDevice.h"

namespace VulkanCore {

    struct ApplicationConfiguration
    {
        const WindowConfiguration WindowConfig;

        // TODO: add

        ApplicationConfiguration(const WindowConfiguration& WindowConfig);
    };

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
        Window MainWindow;
        GPUDevice MainDevice;

        bool bIsRunning; // TODO: de facut o functie Close()
    };

} // namespace VulkanCore
