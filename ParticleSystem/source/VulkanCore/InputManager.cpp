#include "InputManager.h"

#include <iostream>
#include <fstream>
#include <algorithm>

#include "Time.h"

namespace VulkanCore {

	const std::unordered_map<Benchmark, std::string> InputManager::benchmarkToFileName = {
		{ Benchmark::Test1, "benchmark/test-1.json" },
		{ Benchmark::Test2, "benchmark/test-2.json" },
		{ Benchmark::Test3, "benchmark/test-3.json" },
		{ Benchmark::Test4, "benchmark/test-4.json" },
		{ Benchmark::Test5, "benchmark/test-5.json" }
	};

	InputManager::InputManager(Window& window)
		: window(window)
		, benchmarkJSON(std::nullopt)
		, mousePosition(glm::dvec2(0.0, 0.0))
		, mouseButtonLeftPressed(false)
		, timer(0.0f)
		, mousePositionIndex(0)
		, mouseButtonLeftPressedIndex(0)
	{

	}

	InputManager::~InputManager()
	{

	}

	void InputManager::Update(float deltaTime)
	{
		if (benchmarkJSON.has_value())
		{
			timer += deltaTime;

			// Update Mouse Position
			mousePositionIndex = std::min(mousePositionIndex, benchmarkJSON.value()["mousePosition"].size() - 1);
			if (benchmarkJSON.value()["mousePosition"][mousePositionIndex]["interpolate"])
			{
				const auto& startPoint = benchmarkJSON.value()["mousePosition"][mousePositionIndex - 1];
				const auto& endPoint = benchmarkJSON.value()["mousePosition"][mousePositionIndex];

				const double& startTime = startPoint["time"];
				const double& endTime = endPoint["time"];
				const double t = (timer - startTime) / (endTime - startTime);

				const double startX = startPoint["x"];
				const double startY = startPoint["y"];
				const double endX = endPoint["x"];
				const double endY = endPoint["y"];

				mousePosition.x = startX + t * (endX - startX);
				mousePosition.y = startY + t * (endY - startY);
			}

			if (timer >= benchmarkJSON.value()["mousePosition"][mousePositionIndex]["time"])
			{
				mousePosition.x = benchmarkJSON.value()["mousePosition"][mousePositionIndex]["x"];
				mousePosition.y = benchmarkJSON.value()["mousePosition"][mousePositionIndex]["y"];
				++mousePositionIndex;
			}

			// Update Mouse Button Left Pressed
			mouseButtonLeftPressedIndex = std::min(mouseButtonLeftPressedIndex, benchmarkJSON.value()["mouseButtonLeftPressed"].size() - 1);
			if (timer >= benchmarkJSON.value()["mouseButtonLeftPressed"][mouseButtonLeftPressedIndex]["time"])
			{
				mouseButtonLeftPressed = benchmarkJSON.value()["mouseButtonLeftPressed"][mouseButtonLeftPressedIndex]["value"];
				++mouseButtonLeftPressedIndex;
			}

			// The benchmark has ended
			const double mousePositionMaxTime = benchmarkJSON.value()["mousePosition"][benchmarkJSON.value()["mousePosition"].size() - 1]["time"];
			const double mouseButtonLeftPressedMaxTime = benchmarkJSON.value()["mouseButtonLeftPressed"][benchmarkJSON.value()["mouseButtonLeftPressed"].size() - 1]["time"];
			if (timer >= mousePositionMaxTime && timer >= mouseButtonLeftPressedMaxTime)
			{
				// TODO: save + print results
				benchmarkJSON = std::nullopt;
				window.UnblockWindow();
			}
		}
		else
		{
			glfwGetCursorPos(window.GetGLFWWindow(), &mousePosition.x, &mousePosition.y);
			mouseButtonLeftPressed = glfwGetMouseButton(window.GetGLFWWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS ? true : false;
		}
	}

	void InputManager::StartBenchmark(const Benchmark& benchmark)
	{
		if (benchmarkJSON.has_value())
		{
			return;
		}

		window.BlockWindow();

		try
		{
			benchmarkJSON = LoadJSONBenchmarkTest(benchmarkToFileName.at(benchmark));
			timer = 0.0f;
			mousePositionIndex = 1;
			mouseButtonLeftPressedIndex = 1;
		}
		catch (const std::exception& e)
		{
			benchmarkJSON = std::nullopt;
			std::cout << e.what() << std::endl;
		}
	}

	nlohmann::json InputManager::LoadJSONBenchmarkTest(const std::string& fileName) const
	{
		std::ifstream benchmarkFile(fileName);
		if (!benchmarkFile.is_open())
		{
			throw std::runtime_error("ERROR: Could not find " + fileName);
		}

		nlohmann::json json;
		try
		{
			benchmarkFile >> json;
		}
		catch (const nlohmann::json::parse_error& e)
		{
			throw std::runtime_error("ERROR: " + fileName + " is not valid: " + e.what());
		}

		if (!json.contains("mousePosition") || !json["mousePosition"].is_array()
			|| !json.contains("mouseButtonLeftPressed") || !json["mouseButtonLeftPressed"].is_array())
		{
			throw std::runtime_error("ERROR: Invalid input: missing 'mousePosition' array or 'mouseButtonLeftPressed' array");
		}

		benchmarkFile.close();
		return json;
	}

} // namespace VulkanCore
