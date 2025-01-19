#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Engine {

	class Camera
	{
	public:
		enum class CameraMovement
		{
			FORWARD,
			BACKWARD,
			LEFT,
			RIGHT,
			UP,
			DOWN
		};

		// Constructor
		Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);

		// Destructor
		~Camera();

		// Input
		void ProcessKeyboard(CameraMovement direction, float deltaTime);
		void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
		void ProcessMouseScroll(float yoffset);

		// Getters
		inline glm::mat4 getViewMatrix() const { return glm::lookAt(position, position + front, up); }
		inline glm::mat4 getProjectionMatrix(const float width, const float height) const { return glm::perspective(glm::radians(zoom), width / height, 0.1f, 1000.0f); }

	private:
		// camera attributes
		glm::vec3 position;
		glm::vec3 front;
		glm::vec3 up;
		glm::vec3 right;
		glm::vec3 worldUp;

		// euler angles
		float yaw;
		float pitch;

		// camera options
		float movementSpeed;
		float mouseSensitivity;
		float zoom;

		// default camera values
		static const float YAW;
		static const float PITCH;
		static const float SPEED;
		static const float SENSITIVITY;
		static const float ZOOM;

		void UpdateCameraVectors();
	};

} // namespace Engine
