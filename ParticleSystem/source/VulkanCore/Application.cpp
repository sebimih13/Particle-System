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
		, statueTexture(device, "resources/textures/statue.jpg")	// TODO: MOVE
	{
		// TODO: move
		CreateUniformBuffers();

		CreateDescriptorSetLayout();
		CreateDescriptorPool();
		CreateDescriptorSets();
		CreatePipeline();

		SetupImGui();
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
			.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1)			// ubo layout binding
			.AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)	// sampler layout binding
			.Build();
	}

	void Application::CreateDescriptorPool()
	{
		globalPool = DescriptorPool::Builder(device)
			.SetMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT * 2)												// a set of MAX_FRAMES_IN_FLIGHT for ImGui backend
			.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)				// uniform buffer
			.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SwapChain::MAX_FRAMES_IN_FLIGHT * 2)	// sampler + a set of MAX_FRAMES_IN_FLIGHT for ImGui backend fonts
			.AddFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)									// required for ImGui backend
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

} // namespace VulkanCore
