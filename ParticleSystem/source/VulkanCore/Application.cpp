#include "Application.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

// TODO: move
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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

		SetupImGui();

		// TODO: remove
		CreateTextureImage();
	}

	Application::~Application()
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

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
				// Start frame
				renderer.BeginSwapChainRenderPass(commandBuffer);

				// Update ubo
				UpdateUniformBuffer(renderer.GetSwapChain()->GetCurrentFrameIndex());

				// Triangle
				pipeline->Bind(commandBuffer);
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetPipelineLayout(), 0, 1, &globalDescriptorSets[renderer.GetSwapChain()->GetCurrentFrameIndex()], 0, nullptr);
				triangle.Bind(commandBuffer);
				triangle.Draw(commandBuffer);

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
			.SetMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT * 2)					// for ImGui backend
			.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
			.AddFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)		// for ImGui backend
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
		// TODO: test
#if defined(PLATFORM_WINDOWS) || (defined(PLATFORM_LINUX) && defined(NDEBUG))
		static const std::string vertShaderFilePath = "shaders/triangle.vert.spv";
		static const std::string fragShaderFilePath = "shaders/triangle.frag.spv";
#elif defined(PLATFORM_LINUX) && defined(DEBUG)
		static const std::string vertShaderFilePath = "ParticleSystem/shaders/triangle.vert.spv";
		static const std::string fragShaderFilePath = "ParticleSystem/shaders/triangle.frag.spv";
#endif

		pipeline = std::make_unique<Pipeline>(device, renderer.GetSwapChain()->GetRenderPass(), globalSetLayout->GetDescriptorSetLayout(), vertShaderFilePath, fragShaderFilePath);
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
		initInfoImGui.QueueFamily = device.GetPhysicalQueueFamilies().graphicsFamily.value();
		initInfoImGui.Queue = device.GetGraphicsQueue();
		initInfoImGui.DescriptorPool = globalPool->GetDescriptorPool();
		initInfoImGui.RenderPass = renderer.GetSwapChain()->GetRenderPass();
		initInfoImGui.Subpass = 0;
		initInfoImGui.MinImageCount = 2;
		initInfoImGui.ImageCount = SwapChain::MAX_FRAMES_IN_FLIGHT;
		initInfoImGui.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		ImGui_ImplVulkan_Init(&initInfoImGui);

		// TODO
		// beginSingleTimeCommands()
		// ceva cu fonts
	}

	void Application::RenderUI()
	{
		ImGui::ShowDemoWindow();

		// TODO
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

	// TODO: move
	void Application::CreateTextureImage()
	{
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load("textures/statue.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		if (!pixels)
		{
			throw std::runtime_error("Failed to load texture image!");
		}
	}

} // namespace VulkanCore
