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
#include "UserInterface.h"
#include "Time.h"

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
        UserInterface ui;

        bool bIsRunning; // TODO: de facut o functie Close()

        double lastUpdate;
        TimeData time;

        // Particle System Descriptors
        std::unique_ptr<DescriptorPool> particleSystemDescriptorPool;

        std::unique_ptr<DescriptorSetLayout> particleSystemGraphicsDescriptorSetLayout;
        VkDescriptorSet particleSystemGraphicsDescriptorSet;

        std::unique_ptr<DescriptorSetLayout> particleSystemComputeDescriptorSetLayout;
        VkDescriptorSet particleSystemComputeDescriptorSet;

        std::unique_ptr<Pipeline> particleSystemPipeline;

        // Buffers
        VkBuffer uniformBuffer;
        VkDeviceMemory uniformBufferMemory;
        void* uniformBufferMapped;

        VkBuffer shaderStorageBuffer;
        VkDeviceMemory shaderStorageBufferMemory;

        // TODO: delete
        // Texture statueTexture;

        void Update();
        void Tick(const double& deltaTime);
        void Draw();

        void CreateDescriptorPool();
        void CreateDescriptorSetLayout();
        void CreateDescriptorSets();
        void CreatePipeline();

        // Buffers
        void CreateUniformBuffer();
        void UpdateUniformBuffer();
        void CleanupUniformBuffer();

        void CreateShaderStorageBuffer();
        void CleanupShaderStorageBuffer();
    };

} // namespace VulkanCore
