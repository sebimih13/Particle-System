#include "Application.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

// TODO: test
#include <vector>
#include <chrono>
#include <cstring>
#include <stdexcept>
#include <random>
#include <ctime>
#include <iostream>

// TODO: test
#include "Model.h"
#include "Particle.h"

namespace VulkanCore {

	ApplicationConfiguration::ApplicationConfiguration(const WindowConfiguration& windowConfig)
		: windowConfig(windowConfig)
	{

	}

	const uint32_t Application::PARTICLE_COUNT = 131072 * 64; // TODO: 131072 * 64 = 8_388_608

	Application::Application(const ApplicationConfiguration& config)
		: window(config.windowConfig)
		, device(window)
		, renderer(window, device)
		, bIsRunning(true)
		, lastUpdate(0.0)
		// , statueTexture(device, "resources/textures/statue.jpg")	// TODO: DELETE
	{
		lastUpdate = glfwGetTime();

		// Buffers Setup
		CreateShaderStorageBuffer();
		CreateUniformBuffer();

		// Descriptors Setup
		CreateDescriptorPool();
		CreateDescriptorSetLayout();
		CreateDescriptorSets();

		// Pipelines
		CreatePipeline();

		// ImGui Setup
		// SetupImGui();
	}

	Application::~Application()
	{
		// ImGui cleanup
		//ImGui_ImplVulkan_Shutdown();
		//ImGui_ImplGlfw_Shutdown();
		//ImGui::DestroyContext();

		CleanupShaderStorageBuffer();
		CleanupUniformBuffer();
	}

	void Application::Run()
	{
		while (!window.ShouldClose() && bIsRunning)
		{
			window.Update();

			// Empty submission
			renderer.SyncNewFrame();

			Update();
			Draw();
		}

		vkDeviceWaitIdle(device.GetVKDevice());
	}

	void Application::Update()
	{
		// Update Uniform Buffer if window has been resized
		if (window.GetWasWindowResized())
		{
			UpdateUniformBuffer();
		}

		// Update the application, only tick once every 15 milliseconds
		static const uint32_t TICK_MILLIS = 15;
		const double duration = glfwGetTime() - lastUpdate;
		if (duration * 1000.0 >= TICK_MILLIS)
		{
			Tick(duration);
			lastUpdate = glfwGetTime();
		}
	}

	void Application::Tick(const double& deltaTime)
	{
		// Tick the application state based on the wall-clock time since the last tick
		// deltaTime seconds since last frame

		// TODO: doar test
		double xPosMouse, yPosMouse;
		glfwGetCursorPos(window.GetGLFWWindow(), &xPosMouse, &yPosMouse);
		const float world_width = static_cast<float>(window.GetWidth()) / static_cast<float>(window.GetHeight());

		// Update Push-Constants
		PushConstants pushConstantsData = {};
		pushConstantsData.enabled = glfwGetMouseButton(window.GetGLFWWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS ? 1 : 0;
		pushConstantsData.attractor = glm::vec2(
			glm::mix(-world_width, world_width, xPosMouse / window.GetWidth()),
			glm::mix(1.0f, -1.0f, yPosMouse / window.GetHeight())
		);
		pushConstantsData.timestep = static_cast<float>(deltaTime);				// TODO: DOODLE update

		// Compute submission
		if (VkCommandBuffer commandBuffer = renderer.BeginCompute())
		{
			particleSystemPipeline->BindComputePipeline(commandBuffer);
			vkCmdPushConstants(commandBuffer, particleSystemPipeline->GetComputePipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstants), &pushConstantsData);
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, particleSystemPipeline->GetComputePipelineLayout(), 0, 1, &particleSystemComputeDescriptorSet, 0, nullptr);
			vkCmdDispatch(commandBuffer, PARTICLE_COUNT / 64, 1, 1);
		}
		renderer.EndCompute();
	}

	void Application::Draw()
	{
		// Graphics submission
		if (VkCommandBuffer commandBuffer = renderer.BeginFrame()) // TODO: DOODLE: aici trebuie sa schimb logica putin
		{
			// Pipeline Barrier
			{
				VkImageSubresourceRange imageSubresourceRangeInfo = {};
				imageSubresourceRangeInfo.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				imageSubresourceRangeInfo.baseMipLevel = 0;
				imageSubresourceRangeInfo.levelCount = 1;
				imageSubresourceRangeInfo.baseArrayLayer = 0;
				imageSubresourceRangeInfo.layerCount = 1;

				std::array<VkImageMemoryBarrier, 2> imageMemoryBarriers = {};
				imageMemoryBarriers[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				imageMemoryBarriers[0].srcAccessMask = VK_ACCESS_NONE;
				imageMemoryBarriers[0].dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				imageMemoryBarriers[0].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				imageMemoryBarriers[0].newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				imageMemoryBarriers[0].srcQueueFamilyIndex = -1;
				imageMemoryBarriers[0].dstQueueFamilyIndex = -1;
				imageMemoryBarriers[0].image = renderer.GetCurrentIntermediaryImage();
				imageMemoryBarriers[0].subresourceRange = imageSubresourceRangeInfo;

				imageMemoryBarriers[1].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				imageMemoryBarriers[1].srcAccessMask = VK_ACCESS_NONE;
				imageMemoryBarriers[1].dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				imageMemoryBarriers[1].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				imageMemoryBarriers[1].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				imageMemoryBarriers[1].srcQueueFamilyIndex = -1;
				imageMemoryBarriers[1].dstQueueFamilyIndex = -1;
				imageMemoryBarriers[1].image = renderer.GetCurrentSwapchainImage();
				imageMemoryBarriers[1].subresourceRange = imageSubresourceRangeInfo;

				vkCmdPipelineBarrier(
					commandBuffer,
					VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
					VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
					VK_DEPENDENCY_BY_REGION_BIT,
					0, nullptr,
					0, nullptr,
					2, imageMemoryBarriers.data()
				);
			}

			renderer.BeginSwapChainRenderPass(commandBuffer);
			{
				particleSystemPipeline->BindGraphicsPipeline(commandBuffer);
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, particleSystemPipeline->GetGraphicsPipelineLayout(), 0, 1, &particleSystemGraphicsDescriptorSet, 0, nullptr);
				vkCmdDraw(commandBuffer, PARTICLE_COUNT, 1, 0, 0);
			}
			renderer.EndSwapChainRenderPass(commandBuffer);

			// Pipeline Barrier
			{
				VkImageSubresourceRange imageSubresourceRangeInfo = {};
				imageSubresourceRangeInfo.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				imageSubresourceRangeInfo.baseMipLevel = 0;
				imageSubresourceRangeInfo.levelCount = 1;
				imageSubresourceRangeInfo.baseArrayLayer = 0;
				imageSubresourceRangeInfo.layerCount = 1;

				std::array<VkImageMemoryBarrier, 1> imageMemoryBarriers = {};
				imageMemoryBarriers[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				imageMemoryBarriers[0].srcAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				imageMemoryBarriers[0].dstAccessMask = VK_ACCESS_NONE;
				imageMemoryBarriers[0].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				imageMemoryBarriers[0].newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
				imageMemoryBarriers[0].srcQueueFamilyIndex = -1;
				imageMemoryBarriers[0].dstQueueFamilyIndex = -1;
				imageMemoryBarriers[0].image = renderer.GetCurrentSwapchainImage();
				imageMemoryBarriers[0].subresourceRange = imageSubresourceRangeInfo;

				vkCmdPipelineBarrier(
					commandBuffer,
					VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
					VK_DEPENDENCY_BY_REGION_BIT,
					0, nullptr,
					0, nullptr,
					1, imageMemoryBarriers.data()
				);
			}
		}
		renderer.EndFrame(); // TODO: DOODLE SCHIMBA LOGICA
	}

	void Application::CreateDescriptorPool()
	{
		// TODO [PARTICLE-SYSTEM] : test
		//globalPool = DescriptorPool::Builder(device)
		//	.SetMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT * 2)												// + a set of MAX_FRAMES_IN_FLIGHT for ImGui backend
		//	.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)				// uniform buffer
		//	.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SwapChain::MAX_FRAMES_IN_FLIGHT * 2)	// sampler + a set of MAX_FRAMES_IN_FLIGHT for ImGui backend fonts
		//	.AddFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)									// required for ImGui backend
		//	.Build();
			
		particleSystemDescriptorPool = DescriptorPool::Builder(device)
			.SetMaxSets(20 * SwapChain::MAX_FRAMES_IN_FLIGHT)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 * SwapChain::MAX_FRAMES_IN_FLIGHT)				// uniform buffer
			.AddPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 20 * SwapChain::MAX_FRAMES_IN_FLIGHT)			// x2 storage buffers
			.Build();
	}

	void Application::CreateDescriptorSetLayout()
	{
		// TODO [PARTICLE-SYSTEM] : test
		//globalSetLayout = DescriptorSetLayout::Builder(device)
		//	.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1)			// ubo layout binding
		//	.AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)	// sampler layout binding
		//	.Build();

		particleSystemComputeDescriptorSetLayout = DescriptorSetLayout::Builder(device)
			.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1)
			.Build();

		particleSystemGraphicsDescriptorSetLayout = DescriptorSetLayout::Builder(device)
			.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1)
			.AddBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1)
			.Build();
	}

	void Application::CreateDescriptorSets()
	{
		// Descriptor Set for Compute Pipeline
		{
			VkDescriptorBufferInfo storageBufferInfo = {};
			storageBufferInfo.buffer = shaderStorageBuffer;
			storageBufferInfo.offset = 0;
			storageBufferInfo.range = sizeof(Particle) * PARTICLE_COUNT;

			DescriptorWriter(*particleSystemComputeDescriptorSetLayout, *particleSystemDescriptorPool)
				.WriteBuffer(0, storageBufferInfo)
				.Build(particleSystemComputeDescriptorSet);
		}

		// Descriptor Set for Graphics Pipeline
		{
			VkDescriptorBufferInfo uniformBufferInfo = {};
			uniformBufferInfo.buffer = uniformBuffer;
			uniformBufferInfo.offset = 0;
			uniformBufferInfo.range = sizeof(UniformBufferObject);

			VkDescriptorBufferInfo storageBufferInfo = {};
			storageBufferInfo.buffer = shaderStorageBuffer;
			storageBufferInfo.offset = 0;
			storageBufferInfo.range = sizeof(Particle) * PARTICLE_COUNT;

			DescriptorWriter(*particleSystemGraphicsDescriptorSetLayout, *particleSystemDescriptorPool)
				.WriteBuffer(0, uniformBufferInfo)
				.WriteBuffer(1, storageBufferInfo)
				.Build(particleSystemGraphicsDescriptorSet);
		}
	}

	void Application::CreatePipeline()
	{
		// TODO: test
#if defined(PLATFORM_WINDOWS) || (defined(PLATFORM_LINUX) && defined(NDEBUG))
		static const std::string triangleVertShaderFilePath = "shaders/triangle.vert.spv";
		static const std::string triangleFragShaderFilePath = "shaders/triangle.frag.spv";

		static const std::string particleComputeShaderFilePath = "shaders/particle.comp.spv";
		static const std::string particleVertShaderFilePath = "shaders/particle.vert.spv";
		static const std::string particleFragShaderFilePath = "shaders/particle.frag.spv";
#elif defined(PLATFORM_LINUX) && defined(DEBUG)
		static const std::string vertShaderFilePath = "ParticleSystem/shaders/triangle.vert.spv";
		static const std::string fragShaderFilePath = "ParticleSystem/shaders/triangle.frag.spv";

		static const std::string particleComputeShaderFilePath = "ParticleSystem/shaders/particle.comp.spv";
		static const std::string particleVertShaderFilePath = "ParticleSystem/shaders/particle.vert.spv";
		static const std::string particleFragShaderFilePath = "ParticleSystem/shaders/particle.frag.spv";
#endif

		// pipeline = std::make_unique<Pipeline>(device, renderer.GetSwapChain()->GetRenderPass(), globalSetLayout->GetDescriptorSetLayout(), Model::Vertex::GetBindingDescription(), Model::Vertex::GetAttributeDescription(), triangleVertShaderFilePath, triangleFragShaderFilePath);
		particleSystemPipeline = std::make_unique<Pipeline>(device, renderer.GetSwapChain()->GetRenderPass(), particleSystemGraphicsDescriptorSetLayout->GetDescriptorSetLayout(), particleSystemComputeDescriptorSetLayout->GetDescriptorSetLayout(), particleVertShaderFilePath, particleFragShaderFilePath, particleComputeShaderFilePath);
	}

	void Application::SetupImGui()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;		// Enable Keyboard Controls
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;			// Enable Docking
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;			// Enable Multi-Viewport / Platform Windows

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		// Setup Platform/Renderer backends
		//ImGui_ImplGlfw_InitForVulkan(window.GetGLFWWindow(), true);
		//ImGui_ImplVulkan_InitInfo initInfoImGui = {};
		//initInfoImGui.Instance = device.GetInstance();
		//initInfoImGui.PhysicalDevice = device.GetPhysicalDevice();
		//initInfoImGui.Device = device.GetVKDevice();
		//initInfoImGui.QueueFamily = device.GetPhysicalQueueFamilies().graphicsAndComputeFamily.value();
		//initInfoImGui.Queue = device.GetGraphicsQueue();
		//initInfoImGui.DescriptorPool = globalPool->GetDescriptorPool();
		//initInfoImGui.RenderPass = renderer.GetSwapChain()->GetRenderPass();
		//initInfoImGui.Subpass = 0;
		//initInfoImGui.MinImageCount = 2;
		//initInfoImGui.ImageCount = SwapChain::MAX_FRAMES_IN_FLIGHT;
		//initInfoImGui.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		//ImGui_ImplVulkan_Init(&initInfoImGui);
	}

	void Application::RenderUI()
	{
		ImGui::ShowDemoWindow();

		// TODO
	}

	void Application::CreateUniformBuffer()
	{
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		// Create Uniform Buffer
		device.CreateBuffer(
			bufferSize,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			uniformBuffer,
			uniformBufferMemory
		);

		// Map buffer
		vkMapMemory(device.GetVKDevice(), uniformBufferMemory, 0, bufferSize, 0, &uniformBufferMapped);

		// Fill Buffer Data
		UpdateUniformBuffer();
	}

	void Application::UpdateUniformBuffer()
	{
		static constexpr float WORLD_SIZE = 2.0f;
		float aspect = static_cast<float>(window.GetWidth()) / static_cast<float>(window.GetHeight());
		float worldWidth = aspect * WORLD_SIZE;

		UniformBufferObject ubo = {};
		ubo.projection = glm::ortho(
			-worldWidth / 2.0f,
			worldWidth / 2.0f,
			WORLD_SIZE / 2.0f,
			-WORLD_SIZE / 2.0f,
			0.0f,
			-1.0f
		);

		std::memcpy(uniformBufferMapped, &ubo, sizeof(ubo));
	}

	void Application::CleanupUniformBuffer()
	{
		vkDestroyBuffer(device.GetVKDevice(), uniformBuffer, nullptr);
		vkFreeMemory(device.GetVKDevice(), uniformBufferMemory, nullptr);
	}

	void Application::CreateShaderStorageBuffer()
	{
		// Initialize particles positions on a circle
		std::default_random_engine randomEngine(static_cast<unsigned>(std::time(nullptr)));
		std::uniform_real_distribution<float> randomDistribution(0.2f, 1.0f);
		constexpr float step = 2.0f * glm::pi<float>() / static_cast<float>(PARTICLE_COUNT);

		std::vector<Particle> particles(PARTICLE_COUNT);
		for (size_t i = 0; i < particles.size(); ++i)
		{
			const float radius = randomDistribution(randomEngine);
			const float angle = static_cast<float>(i) * step;

			particles[i].position = glm::vec2(radius * glm::cos(angle), radius * glm::sin(angle));
			particles[i].velocity = glm::vec2(0.0f, 0.0f);
			particles[i].color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
		}

		VkDeviceSize bufferSize = sizeof(Particle) * PARTICLE_COUNT;

		// Create a staging buffer used to upload data to the GPU
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		device.CreateBuffer(
			bufferSize, 
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			stagingBuffer, 
			stagingBufferMemory
		);

		// Filling staging buffer
		void* data;
		vkMapMemory(device.GetVKDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
			std::memcpy(data, particles.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(device.GetVKDevice(), stagingBufferMemory);

		// Create Shader Storage Buffer
		device.CreateBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			shaderStorageBuffer, 
			shaderStorageBufferMemory
		);
			
		// Copy initial particle data to storage buffer
		device.CopyBuffer(stagingBuffer, shaderStorageBuffer, bufferSize, device.GetComputeQueue());

		// Cleanup
		vkDestroyBuffer(device.GetVKDevice(), stagingBuffer, nullptr);
		vkFreeMemory(device.GetVKDevice(), stagingBufferMemory, nullptr);
	}

	void Application::CleanupShaderStorageBuffer()
	{
		vkDestroyBuffer(device.GetVKDevice(), shaderStorageBuffer, nullptr);
		vkFreeMemory(device.GetVKDevice(), shaderStorageBufferMemory, nullptr);
	}

} // namespace VulkanCore
