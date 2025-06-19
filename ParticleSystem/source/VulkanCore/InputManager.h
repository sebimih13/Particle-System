#pragma once

#include <string>
#include <unordered_map>
#include <optional>

#include <nlohmann/json.hpp>

#include "Window.h"
#include "Benchmark.h"

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

		void Update(float deltaTime);
		void StartBenchmark(const Benchmark& benchmark);

		// Getters
		inline const glm::dvec2& GetMousePosition() const { return mousePosition; }
		inline const bool GetMouseButtonLeftPressed() const { return mouseButtonLeftPressed; }
		inline const bool GetIsInBenchmark() const { return benchmarkJSON.has_value(); }

	private:
		Window& window;
		std::optional<nlohmann::json> benchmarkJSON;

		glm::dvec2 mousePosition;
		bool mouseButtonLeftPressed;

		float timer;
		size_t mousePositionIndex;
		size_t mouseButtonLeftPressedIndex;

		const static std::unordered_map<Benchmark, std::string> benchmarkToFileName;

		nlohmann::json LoadJSONBenchmarkTest(const std::string& fileName) const;
	};

} // namespace VulkanCore
