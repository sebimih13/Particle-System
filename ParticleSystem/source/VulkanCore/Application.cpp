#include "Application.h"

namespace VulkanCore {

	ApplicationConfiguration::ApplicationConfiguration(const WindowConfiguration& WindowConfig)
		: WindowConfig(WindowConfig)
	{

	}

	Application::Application(const ApplicationConfiguration& config)
		: MainWindow(config.WindowConfig)
		, MainDevice()
		, bIsRunning(true)
	{

	}

	Application::~Application()
	{

	}

	void Application::Run()
	{
		while (MainWindow.ShouldClose() && bIsRunning)
		{
			// TODO

			MainWindow.Update();
		}
	}

} // namespace VulkanCore
