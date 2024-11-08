#include "Application.h"

namespace VulkanCore {

	ApplicationConfiguration::ApplicationConfiguration(const WindowConfiguration& windowConfig)
		: windowConfig(windowConfig)
	{

	}

	Application::Application(const ApplicationConfiguration& config)
		: window(config.windowConfig)
		, device()
		, bIsRunning(true)
	{

	}

	Application::~Application()
	{

	}

	void Application::Run()
	{
		while (window.ShouldClose() && bIsRunning)
		{
			// TODO

			window.Update();
		}
	}

} // namespace VulkanCore
