#pragma once

#include <unordered_map>

// TODO: Forward Declarations
#include "Window.h"

namespace VulkanCore {

	class InputManager
	{
	public:
		// Constructors
		InputManager(Window& window);

		// Destructor
		~InputManager();

		// Not copyable
		InputManager(const InputManager&) = delete;
		InputManager& operator = (const InputManager&) = delete;

		// Not moveable
		InputManager(InputManager&&) = delete;
		InputManager& operator = (InputManager&&) = delete;

		void Update();

		inline const glm::dvec2& getMousePosition() const { return mousePosition; }
		inline const bool& getMouseButtonLeftPressed() const { return mouseButtonLeftPressed; }

	private:
		Window& window;

		glm::dvec2 mousePosition;
		bool mouseButtonLeftPressed;
	};

} // namespace VulkanCore
