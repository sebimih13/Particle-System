#include "InputManager.h"

namespace VulkanCore {

	InputManager::InputManager(Window& window)
		: window(window)
	{

	}

	InputManager::~InputManager()
	{

	}

	void InputManager::Update()
	{
		glfwGetCursorPos(window.GetGLFWWindow(), &mousePosition.x, &mousePosition.y);
		mouseButtonLeftPressed = glfwGetMouseButton(window.GetGLFWWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS ? true : false;
	}

} // namespace VulkanCore
