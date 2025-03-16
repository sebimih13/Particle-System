#pragma once

// TODO: delete - test
#include <vector>
#include <memory>

// TODO: Forward Declarations
#include "Window.h"
#include "GPUDevice.h"
#include "Renderer.h"
#include "Descriptor.h"
#include "Texture.h"

namespace VulkanCore {

    struct ApplicationConfiguration
    {
        const WindowConfiguration windowConfig;

        // TODO: add

        ApplicationConfiguration(const WindowConfiguration& windowConfig);
    };

    // TODO: move FrameInfo.h
    struct UniformBufferObject
    {
        glm::mat4 projection;
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
        static const uint32_t PARTICLE_COUNT;

        Window window;
        GPUDevice device;
        Renderer renderer;

        bool bIsRunning; // TODO: de facut o functie Close()

        // TODO [PARTICLE-SYSTEM] : test
        // Global Descriptors
        //std::unique_ptr<DescriptorSetLayout> globalSetLayout;
        //std::unique_ptr<DescriptorPool> globalPool;
        //std::vector<VkDescriptorSet> globalDescriptorSets;
        //std::unique_ptr<Pipeline> pipeline;

        double lastTime;
        float lastFrameTime;

        // Particle System Descriptors
        std::unique_ptr<DescriptorPool> particleSystemDescriptorPool;

        std::unique_ptr<DescriptorSetLayout> particleSystemGraphicsDescriptorSetLayout;
        std::vector<VkDescriptorSet> particleSystemGraphicsDescriptorSets;

        std::unique_ptr<DescriptorSetLayout> particleSystemComputeDescriptorSetLayout;
        std::vector<VkDescriptorSet> particleSystemComputeDescriptorSets;

        std::unique_ptr<Pipeline> particleSystemPipeline;

        Texture statueTexture;

        void UpdateUniformBuffer(uint32_t currentFrame);

        void CreateDescriptorPool();
        void CreateDescriptorSetLayout();
        void CreateDescriptorSets();
        void CreatePipeline();
        void SetupImGui();

        void RenderUI();

        // TODO: REFACTOR - use Buffer class
        std::vector<VkBuffer> uniformBuffers;
        std::vector<VkDeviceMemory> uniformBuffersMemory;
        std::vector<void*> uniformBuffersMapped;

        void CreateUniformBuffers();
        void CleanupUniformBuffers();

        // TODO: REFACTOR - use Buffer class
        std::vector<VkBuffer> shaderStorageBuffers;
        std::vector<VkDeviceMemory> shaderStorageBuffersMemory;

        void CreateShaderStorageBuffers();
        void CleanupShaderStorageBuffers();
    };

} // namespace VulkanCore
