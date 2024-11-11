#include "Application.h"

namespace VulkanCore {

	ApplicationConfiguration::ApplicationConfiguration(const WindowConfiguration& windowConfig)
		: windowConfig(windowConfig)
	{

	}

	Application::Application(const ApplicationConfiguration& config)
		: window(config.windowConfig)
		, device(window)
		, swapChain(device, window)
		, graphicsPipeline(device, swapChain)
		, renderer(window, device, swapChain, graphicsPipeline)
		, bIsRunning(true)
	{

	}

	Application::~Application()
	{

	}

	void Application::Run()
	{
		while (!window.ShouldClose() && bIsRunning)
		{
			window.Update();

			// draw
			renderer.DrawFrame();

			// TODO
		}

		vkDeviceWaitIdle(device.GetVKDevice());
	}

} // namespace VulkanCore
