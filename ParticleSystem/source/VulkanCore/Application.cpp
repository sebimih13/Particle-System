#include "Application.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// TODO: test
#include <vector>	// TODO: remove
#include <chrono>
#include <cstring>

// TODO: test
#include "Model.h"

namespace VulkanCore {

	ApplicationConfiguration::ApplicationConfiguration(const WindowConfiguration& windowConfig)
		: windowConfig(windowConfig)
	{

	}

	Application::Application(const ApplicationConfiguration& config)
		: window(config.windowConfig)
		, device(window)
		, renderer(window, device)
		, bIsRunning(true)
	{
		// TODO: move
		CreateUniformBuffers();

		CreateDescriptorSetLayout();
		CreateDescriptorPool();
		CreateDescriptorSets();
		CreatePipeline();
	}

	Application::~Application()
	{
		// TODO: move
		CleanupUniformBuffers();
	}

	void Application::Run()
	{
		// TODO: test
		const std::vector<Model::Vertex> vertices = {
			Model::Vertex(glm::vec2(-0.5f, -0.5f),	glm::vec3(1.0f, 0.0f, 0.0f)),
			Model::Vertex(glm::vec2( 0.5f, -0.5f),	glm::vec3(0.0f, 1.0f, 0.0f)),
			Model::Vertex(glm::vec2( 0.5f,  0.5f),	glm::vec3(0.0f, 0.0f, 1.0f)),
			Model::Vertex(glm::vec2(-0.5f,  0.5f),	glm::vec3(1.0f, 1.0f, 1.0f))
		};

		const std::vector<uint32_t> indices = {
			0, 1, 2,
			2, 3, 0
		};

		Model::Data triangleData(vertices, indices);
		Model triangle(device, triangleData);

		while (!window.ShouldClose() && bIsRunning)
		{
			window.Update();

			// draw
			if (VkCommandBuffer commandBuffer = renderer.BeginFrame())
			{
				renderer.BeginSwapChainRenderPass(commandBuffer);

				// update ubo
				UpdateUniformBuffer(renderer.GetSwapChain()->GetCurrentFrameIndex());

				// triangle
				pipeline->Bind(commandBuffer);
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetPipelineLayout(), 0, 1, &globalDescriptorSets[renderer.GetSwapChain()->GetCurrentFrameIndex()], 0, nullptr);
				triangle.Bind(commandBuffer);
				triangle.Draw(commandBuffer);

				renderer.EndSwapChainRenderPass(commandBuffer);
				renderer.EndFrame();
			}

			// TODO
		}

		vkDeviceWaitIdle(device.GetVKDevice());
	}

	void Application::UpdateUniformBuffer(uint32_t currentFrame)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		UniformBufferObject ubo = {};
		ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(renderer.GetSwapChain()->GetSwapChainExtent().width) / static_cast<float>(renderer.GetSwapChain()->GetSwapChainExtent().height), 0.1f, 10.0f);
		ubo.proj[1][1] *= -1;

		std::memcpy(uniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));
	}

	void Application::CreateDescriptorSetLayout()
	{
		globalSetLayout = DescriptorSetLayout::Builder(device)
			.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1)
			.Build();
	}

	void Application::CreateDescriptorPool()
	{
		globalPool = DescriptorPool::Builder(device)
			.SetMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3)
			.Build();
	}

	void Application::CreateDescriptorSets()
	{
		globalDescriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; ++i)
		{
			globalPool->AllocateDescriptor(globalSetLayout->GetDescriptorSetLayout(), globalDescriptorSets[i]);

			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = uniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);

			VkWriteDescriptorSet descriptorWrite = {};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = globalDescriptorSets[i];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;
			descriptorWrite.pImageInfo = nullptr;			// optional
			descriptorWrite.pTexelBufferView = nullptr;		// optional

			vkUpdateDescriptorSets(device.GetVKDevice(), 1, &descriptorWrite, 0, nullptr);
		}
	}

	void Application::CreatePipeline()
	{
		pipeline = std::make_unique<Pipeline>(device, renderer.GetSwapChain()->GetRenderPass(), globalSetLayout->GetDescriptorSetLayout());
	}

	// TODO: delete
	void Application::CreateUniformBuffers()
	{
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		uniformBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		uniformBuffersMemory.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		uniformBuffersMapped.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; ++i)
		{
			device.CreateBuffer(
				bufferSize,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				uniformBuffers[i],
				uniformBuffersMemory[i]
			);

			vkMapMemory(device.GetVKDevice(), uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
		}
	}

	// TODO: delete
	void Application::CleanupUniformBuffers()
	{
		for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; ++i)
		{
			vkDestroyBuffer(device.GetVKDevice(), uniformBuffers[i], nullptr);
			vkFreeMemory(device.GetVKDevice(), uniformBuffersMemory[i], nullptr);
		}
	}

} // namespace VulkanCore
