#pragma once

// TODO: delete - test
#include <vector>
#include <memory>

// TODO: Forward Declarations
#include "Window.h"
#include "GPUDevice.h"
#include "Renderer.h"
#include "Descriptor.h"

namespace VulkanCore {

    struct ApplicationConfiguration
    {
        const WindowConfiguration windowConfig;

        // TODO: add

        ApplicationConfiguration(const WindowConfiguration& windowConfig);
    };

    // TODO: move FrameInfo.h
    struct UniformBufferObject {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
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
        Renderer renderer;

        bool bIsRunning; // TODO: de facut o functie Close()

        std::unique_ptr<DescriptorSetLayout> globalSetLayout;
        std::unique_ptr<DescriptorPool> globalPool;
        std::vector<VkDescriptorSet> globalDescriptorSets;
        std::unique_ptr<Pipeline> pipeline;

        void UpdateUniformBuffer(uint32_t currentFrame);

        void CreateDescriptorSetLayout();
        void CreateDescriptorPool();
        void CreateDescriptorSets();
        void CreatePipeline();
        void SetupImGui();

        // TODO: REFACTOR - use Buffer class
        void CreateUniformBuffers();
        void CleanupUniformBuffers();

        // TODO: delete
        std::vector<VkBuffer> uniformBuffers;
        std::vector<VkDeviceMemory> uniformBuffersMemory;
        std::vector<void*> uniformBuffersMapped;
    };

} // namespace VulkanCore
