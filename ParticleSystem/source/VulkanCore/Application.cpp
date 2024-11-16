#include "Application.h"

// TODO: test
#include "Model.h"

// TODO: test
#include <vector>

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

	}

	Application::~Application()
	{

	}

	void Application::Run()
	{
		// TODO: test
		const std::vector<Model::Vertex> vertices = {
			Model::Vertex(glm::vec2( 0.0f, -0.5f),	glm::vec3(1.0f, 1.0f, 1.0f)),
			Model::Vertex(glm::vec2( 0.5f,  0.5f),	glm::vec3(0.0f, 1.0f, 0.0f)),
			Model::Vertex(glm::vec2(-0.5f,  0.5f),	glm::vec3(0.0f, 0.0f, 1.0f))
		};

		Model::Data triangleData(vertices);
		Model triangle(device, triangleData);

		while (!window.ShouldClose() && bIsRunning)
		{
			window.Update();

			// draw
			if (VkCommandBuffer commandBuffer = renderer.BeginFrame())
			{
				renderer.BeginSwapChainRenderPass(commandBuffer);

				triangle.Bind(commandBuffer);
				triangle.Draw(commandBuffer);

				renderer.EndSwapChainRenderPass(commandBuffer);
				renderer.EndFrame();
			}

			// TODO
		}

		vkDeviceWaitIdle(device.GetVKDevice());
	}

} // namespace VulkanCore
