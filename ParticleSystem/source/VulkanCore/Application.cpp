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

	const uint32_t Application::PARTICLE_COUNT = 8192;

	Application::Application(const ApplicationConfiguration& config)
		: window(config.windowConfig)
		, device(window)
		, renderer(window, device)
		, bIsRunning(true)
		, statueTexture(device, "resources/textures/statue.jpg")	// TODO: MOVE
	{
		// Buffers Setup
		// TODO: REFACTOR - use Buffer class
		CreateUniformBuffers();
		CreateShaderStorageBuffers();

		// Descriptors Setup
		CreateDescriptorSetLayout();
		CreateDescriptorPool();
		CreateDescriptorSets();

		// Pipelines
		CreatePipeline();

		// ImGui Setup
		SetupImGui();
	}

	Application::~Application()
	{
		// ImGui cleanup
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		// TODO: REFACTOR - use Buffer class
		CleanupShaderStorageBuffers();
		CleanupUniformBuffers();
	}

	void Application::Run()
	{
		// TODO: test Model
		const std::vector<Model::Vertex> vertices = {
			Model::Vertex(glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)),
			Model::Vertex(glm::vec3( 0.5f, -0.5f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)),
			Model::Vertex(glm::vec3( 0.5f,  0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)),
			Model::Vertex(glm::vec3(-0.5f,  0.5f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f)),

			Model::Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)),
			Model::Vertex(glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)),
			Model::Vertex(glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)),
			Model::Vertex(glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f))
		};

		const std::vector<uint32_t> indices = {
			0, 1, 2,
			2, 3, 0,

			4, 5, 6, 
			6, 7, 4
		};

		Model triangle(device, Model::Data(vertices, indices));

		while (!window.ShouldClose() && bIsRunning)
		{
			window.Update();

			// Compute submission
			//if (VkCommandBuffer commandBuffer = renderer.BeginCompute())
			//{
			//	// Update UBO
			//	UpdateUniformBuffer(renderer.GetSwapChain()->GetCurrentFrameIndex());

			//	// Particle System
			//	particleSystemPipeline->BindCompute(commandBuffer);
			//	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, particleSystemPipeline->GetComputePipelineLayout(), 0, 1, &particleSystemDescriptorSets[renderer.GetSwapChain()->GetCurrentFrameIndex()], 0, nullptr);
			//	vkCmdDispatch(commandBuffer, PARTICLE_COUNT / 256, 1, 1);
			//}
			//renderer.EndCompute();

			// Graphics submission
			if (VkCommandBuffer commandBuffer = renderer.BeginFrame())
			{
				// Start frame
				renderer.BeginSwapChainRenderPass(commandBuffer);

				// Update ubo
				// UpdateUniformBuffer(renderer.GetSwapChain()->GetCurrentFrameIndex());

				// Triangle
				//pipeline->BindGraphicsPipeline(commandBuffer);
				//vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetGraphicsPipelineLayout(), 0, 1, &globalDescriptorSets[renderer.GetSwapChain()->GetCurrentFrameIndex()], 0, nullptr);
				//triangle.Bind(commandBuffer);
				//triangle.Draw(commandBuffer);

				// Particle System
				particleSystemPipeline->BindGraphicsPipeline(commandBuffer);

				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(commandBuffer, 0, 1, &shaderStorageBuffers[renderer.GetSwapChain()->GetCurrentFrameIndex()], offsets);

				vkCmdDraw(commandBuffer, PARTICLE_COUNT, 1, 0, 0);

				// Start the Dear ImGui frame
				ImGui_ImplVulkan_NewFrame();
				ImGui_ImplGlfw_NewFrame();
				ImGui::NewFrame();

				RenderUI();

				ImGui::Render();
				ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

				// Update and Render additional Platform Windows
				if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
				{
					ImGui::UpdatePlatformWindows();
					ImGui::RenderPlatformWindowsDefault();
				}

				// End frame
				renderer.EndSwapChainRenderPass(commandBuffer);
			}
			renderer.EndFrame();
		}

		vkDeviceWaitIdle(device.GetVKDevice());
	}

	void Application::UpdateUniformBuffer(uint32_t currentFrame)
	{
		//static auto startTime = std::chrono::high_resolution_clock::now();
		//auto currentTime = std::chrono::high_resolution_clock::now();
		//float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		//UniformBufferObject ubo = {};
		//ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		//ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		//ubo.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(renderer.GetSwapChain()->GetSwapChainExtent().width) / static_cast<float>(renderer.GetSwapChain()->GetSwapChainExtent().height), 0.1f, 10.0f);
		//ubo.proj[1][1] *= -1;

		static double lastTime = 0.0f;
		double currentTime = glfwGetTime();
		float lastFrameTime = static_cast<float>(currentTime - lastTime) * 1000.0f;
		lastTime = currentTime;

		UniformBufferObject ubo = {};
		ubo.deltaTime = lastFrameTime * 2.0f;

		std::memcpy(uniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));
	}

	void Application::CreateDescriptorSetLayout()
	{
		globalSetLayout = DescriptorSetLayout::Builder(device)
			.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1)			// ubo layout binding
			.AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)	// sampler layout binding
			.Build();

		particleSystemSetLayout = DescriptorSetLayout::Builder(device)
			.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1)
			.AddBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1)
			.AddBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1)
			.Build();
	}

	void Application::CreateDescriptorPool()
	{
		globalPool = DescriptorPool::Builder(device)
			.SetMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT * 2)												// + a set of MAX_FRAMES_IN_FLIGHT for ImGui backend
			.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)				// uniform buffer
			.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SwapChain::MAX_FRAMES_IN_FLIGHT * 2)	// sampler + a set of MAX_FRAMES_IN_FLIGHT for ImGui backend fonts
			.AddFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)									// required for ImGui backend
			.Build();
			
		particleSystemDescriptorPool = DescriptorPool::Builder(device)
			.SetMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)				// uniform buffer
			.AddPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2 * SwapChain::MAX_FRAMES_IN_FLIGHT)			// x2 storage buffers
			.Build();
	}

	void Application::CreateDescriptorSets()
	{
		globalDescriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; ++i)
		{
			// TODO: REFACTOR: de pus intr-o clasa Buffer->GetDescriptorInfo()
			VkDescriptorBufferInfo uboBufferInfo = {};
			uboBufferInfo.buffer = uniformBuffers[i];
			uboBufferInfo.offset = 0;
			uboBufferInfo.range = sizeof(UniformBufferObject);

			// TODO: REFACTOR
			VkDescriptorImageInfo imageInfo = {};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = statueTexture.GetTextureImageView();
			imageInfo.sampler = statueTexture.GetTextureSampler();

			DescriptorWriter(*globalSetLayout, *globalPool)
				.WriteBuffer(0, uboBufferInfo)
				.WriteImage(1, imageInfo)
				.Build(globalDescriptorSets[i]);
		}

		particleSystemDescriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; ++i)
		{
			VkDescriptorBufferInfo uniformBufferInfo = {};
			uniformBufferInfo.buffer = uniformBuffers[i];
			uniformBufferInfo.offset = 0;
			uniformBufferInfo.range = sizeof(UniformBufferObject);

			VkDescriptorBufferInfo storageBufferInfoLastFrame = {};
			storageBufferInfoLastFrame.buffer = shaderStorageBuffers[(i - 1) % SwapChain::MAX_FRAMES_IN_FLIGHT];
			storageBufferInfoLastFrame.offset = 0;
			storageBufferInfoLastFrame.range = sizeof(Particle) * PARTICLE_COUNT;

			VkDescriptorBufferInfo storageBufferInfoCurrentFrame = {};
			storageBufferInfoCurrentFrame.buffer = shaderStorageBuffers[i];
			storageBufferInfoCurrentFrame.offset = 0;
			storageBufferInfoCurrentFrame.range = sizeof(Particle) * PARTICLE_COUNT;

			DescriptorWriter(*particleSystemSetLayout, *particleSystemDescriptorPool)
				.WriteBuffer(0, uniformBufferInfo)
				.WriteBuffer(1, storageBufferInfoLastFrame)
				.WriteBuffer(2, storageBufferInfoCurrentFrame)
				.Build(particleSystemDescriptorSets[i]);
		}
	}

	void Application::CreatePipeline()
	{
		// TODO: test
#if defined(PLATFORM_WINDOWS) || (defined(PLATFORM_LINUX) && defined(NDEBUG))
		static const std::string triangleVertShaderFilePath = "shaders/triangle.vert.spv";
		static const std::string triangleFragShaderFilePath = "shaders/triangle.frag.spv";

		static const std::string particleVertShaderFilePath = "shaders/particle.vert.spv";
		static const std::string particleFragShaderFilePath = "shaders/particle.frag.spv";
		static const std::string particleComputeShaderFilePath = "shaders/particle.comp.spv";
#elif defined(PLATFORM_LINUX) && defined(DEBUG)
		static const std::string vertShaderFilePath = "ParticleSystem/shaders/triangle.vert.spv";
		static const std::string fragShaderFilePath = "ParticleSystem/shaders/triangle.frag.spv";
#endif

		pipeline = std::make_unique<Pipeline>(device, renderer.GetSwapChain()->GetRenderPass(), globalSetLayout->GetDescriptorSetLayout(), Model::Vertex::GetBindingDescription(), Model::Vertex::GetAttributeDescription(), triangleVertShaderFilePath, triangleFragShaderFilePath);
		particleSystemPipeline = std::make_unique<Pipeline>(device, renderer.GetSwapChain()->GetRenderPass(), particleSystemSetLayout->GetDescriptorSetLayout(), Particle::GetBindingDescription(), Particle::GetAttributeDescription(), particleVertShaderFilePath, particleFragShaderFilePath, particleComputeShaderFilePath);
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
		ImGui_ImplGlfw_InitForVulkan(window.GetGLFWWindow(), true);
		ImGui_ImplVulkan_InitInfo initInfoImGui = {};
		initInfoImGui.Instance = device.GetInstance();
		initInfoImGui.PhysicalDevice = device.GetPhysicalDevice();
		initInfoImGui.Device = device.GetVKDevice();
		initInfoImGui.QueueFamily = device.GetPhysicalQueueFamilies().graphicsAndComputeFamily.value();
		initInfoImGui.Queue = device.GetGraphicsQueue();
		initInfoImGui.DescriptorPool = globalPool->GetDescriptorPool();
		initInfoImGui.RenderPass = renderer.GetSwapChain()->GetRenderPass();
		initInfoImGui.Subpass = 0;
		initInfoImGui.MinImageCount = 2;
		initInfoImGui.ImageCount = SwapChain::MAX_FRAMES_IN_FLIGHT;
		initInfoImGui.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		ImGui_ImplVulkan_Init(&initInfoImGui);
	}

	void Application::RenderUI()
	{
		ImGui::ShowDemoWindow();

		// TODO
	}

	// TODO: REFACTOR - use Buffer class
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

	// TODO: REFACTOR - use Buffer class
	void Application::CleanupUniformBuffers()
	{
		for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; ++i)
		{
			vkDestroyBuffer(device.GetVKDevice(), uniformBuffers[i], nullptr);
			vkFreeMemory(device.GetVKDevice(), uniformBuffersMemory[i], nullptr);
		}
	}

	// TODO: REFACTOR - use Buffer class
	void Application::CreateShaderStorageBuffers()
	{
		// Initialize particles positions on a circle
		std::default_random_engine rndEngine(static_cast<unsigned>(std::time(nullptr)));
		std::uniform_real_distribution<float> rndDist(0.0f, 1.0f);

		std::vector<Particle> particles(PARTICLE_COUNT);
		for (auto& particle : particles)
		{
			const float r = 0.25f * sqrt(rndDist(rndEngine));
			const float theta = rndDist(rndEngine) * 2.0f * 3.14159265358979323846f;
			const float x = r * cos(theta) * window.GetHeight() / window.GetHeight();
			const float y = r * sin(theta);

			particle.position = glm::vec2(x, y);
			particle.velocity = glm::normalize(glm::vec2(x, y)) * 0.00025f;
			particle.color = glm::vec4(rndDist(rndEngine), rndDist(rndEngine), rndDist(rndEngine), 1.0f);
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
			std::memcpy(data, particles.data(), (size_t)bufferSize);
		vkUnmapMemory(device.GetVKDevice(), stagingBufferMemory);

		// Create shader storage buffer
		shaderStorageBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		shaderStorageBuffersMemory.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

		// Copy initial particle data to all storage buffers
		for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; ++i)
		{
			device.CreateBuffer(
				bufferSize, 
				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
				shaderStorageBuffers[i], 
				shaderStorageBuffersMemory[i]
			);
			
			device.CopyBuffer(stagingBuffer, shaderStorageBuffers[i], bufferSize);
		}

		vkDestroyBuffer(device.GetVKDevice(), stagingBuffer, nullptr);
		vkFreeMemory(device.GetVKDevice(), stagingBufferMemory, nullptr);
	}

	// TODO: REFACTOR - use Buffer class
	void Application::CleanupShaderStorageBuffers()
	{
		for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; ++i)
		{
			vkDestroyBuffer(device.GetVKDevice(), shaderStorageBuffers[i], nullptr);
			vkFreeMemory(device.GetVKDevice(), shaderStorageBuffersMemory[i], nullptr);
		}
	}

} // namespace VulkanCore
