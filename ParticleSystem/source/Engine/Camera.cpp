#include "Camera.h"

#include <algorithm>

namespace Engine
{
	const float Camera::YAW = -90.0f;
	const float Camera::PITCH = 0.0f;
	const float Camera::SPEED = 2.5f;
	const float Camera::SENSITIVITY = 0.1f;
	const float Camera::ZOOM = 45.0f;

	Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
		: position(position), worldUp(up), yaw(yaw), pitch(pitch)
	{
		UpdateCameraVectors();
	}

	Camera::~Camera()
	{
		// default
	}

	void Camera::ProcessKeyboard(CameraMovement direction, float deltaTime)
	{
		float velocity = movementSpeed * deltaTime;

		switch (direction)
		{
		case Engine::Camera::CameraMovement::FORWARD:
			position += front * velocity;
			break;

		case Engine::Camera::CameraMovement::BACKWARD:
			position -= front * velocity;
			break;

		case Engine::Camera::CameraMovement::LEFT:
			position -= right * velocity;
			break;

		case Engine::Camera::CameraMovement::RIGHT:
			position += right * velocity;
			break;

		case Engine::Camera::CameraMovement::UP:
			position += worldUp * velocity;
			break;

		case Engine::Camera::CameraMovement::DOWN:
			position -= worldUp * velocity;
			break;

		default:
			break;
		}
	}

	void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch)
	{
		xoffset *= mouseSensitivity;
		yoffset *= mouseSensitivity;

		yaw += xoffset;
		pitch += yoffset;

		// make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			pitch = std::max(-89.0f, pitch);
			pitch = std::min(89.0f, pitch);
		}

		UpdateCameraVectors();
	}

	void Camera::ProcessMouseScroll(float yoffset)
	{
		zoom -= yoffset;
		zoom = std::max(1.0f, zoom);
		zoom = std::min(45.0f, zoom);
	}


	void Camera::UpdateCameraVectors()
	{
		// Calculate the new Front vector
		glm::vec3 newFront;
		newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		newFront.y = sin(glm::radians(pitch));
		newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		front = glm::normalize(newFront);

		// Re-calculate the Right and Up vector
		right = glm::normalize(glm::cross(front, worldUp));
		up = glm::normalize(glm::cross(right, front));
	}

} // namespace Engine
