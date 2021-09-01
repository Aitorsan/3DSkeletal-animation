#pragma once
#include "camera.h"
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/matrix_inverse.hpp>
#include <GLFW/glfw3.h>
#include <iostream>

Camera::Camera()
	: Camera(0.0f, 0.0f, 0.0f, 45.0f)
{
}

Camera::Camera(float x, float y, float z, float fov)
	: CameraPos{x,y,z}
	, CameraFront{ 0, 0,0.0f }
	, CameraUp{ 0, 1, 0 }
	, yaw{CameraFront.z > 0 ? 90.f : -90}
	, pitch{0.0f}
	, lastX{}
	, lastY{}
	, init{true}
	, Distance{60.0f}
{

}
void Camera::CheckMouseMovement(GLFWwindow& window)
{
	double xpos{ 0 }, ypos{ 0 };
	glfwGetCursorPos(&window, &xpos, &ypos);
	static bool pressed = false;

	float xoffset = lastX - xpos;
	float yoffset = ypos - lastY;
	lastX = xpos;
	lastY = ypos;
	if (glfwGetMouseButton(&window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && !pressed)
	{
		float sensitivity = 0.1f;
		xoffset *= sensitivity;
		yoffset *= sensitivity;
		yaw += xoffset;
		pitch += yoffset;
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;
		
		glm::vec3 direction{};
		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		CameraFront = glm::normalize(direction);
			
	}
	else if (glfwGetMouseButton(&window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE && pressed)
	{
			pressed = false;
	}
}

void Camera::CheckMouseMovement(GLFWwindow& window,const glm::vec3& target)
{
		glm::vec3 pos{};
		double xpos{ 0 }, ypos{ 0 };
		glfwGetCursorPos(&window, &xpos, &ypos);
		if (!init)
		{
			float xoffset = lastX - xpos ;
			float yoffset = ypos - lastY;
			lastX = xpos;
			lastY = ypos;

			float sensitivity = 0.1f;
			xoffset *= sensitivity;
			yoffset *= sensitivity;


			if (glfwGetMouseButton(&window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
			{
				yaw += xoffset;
				pitch += yoffset;
				if (pitch > 89.0f)
					pitch = 89.0f;
				if (pitch < -89.0f)
					pitch = -89.0f;
			}

		}
		init = false;

		pos.x = Distance * cos(glm::radians(pitch)) * sin(glm::radians(yaw));
		pos.y = Distance * sin(glm::radians(pitch));
		pos.z = Distance * cos(glm::radians(yaw));
		CameraPos.x = target.x - pos.x;
		CameraPos.y = target.y + pos.y;
		CameraPos.z = target.z - pos.z;

		CameraFront = glm::normalize(target - CameraPos);

}


void Camera::MoveFront(float elapsedTime, float velocity)
{

	CameraPos += CameraFront * velocity*elapsedTime;
}

void Camera::MoveBack(float elapsedTime, float velocity)
{
	CameraPos -= CameraFront * velocity*elapsedTime;
}

void Camera::MoveRight(float elapsedTime, float velocity)
{
	CameraPos += glm::normalize(glm::cross(CameraFront, CameraUp)) * elapsedTime*velocity;
}

void Camera::MoveLeft(float elapsedTime, float velocity)
{
	CameraPos -= glm::normalize(glm::cross(CameraFront, CameraUp)) * elapsedTime*velocity;
}

glm::mat4 Camera::GetCameraTranslationMatrix()
{
	return glm::lookAt(CameraPos, CameraPos + CameraFront, CameraUp);
}

glm::vec3& Camera::GetCameraPosition()
{
	return CameraPos;
}

glm::vec3& Camera::GetCameraFront()
{
	return CameraFront;
}

void Camera::setCameraFront(const glm::vec3& front)
{
	CameraFront = front;

}

void Camera::SetCameraPosition(float x, float y, float z)
{
	CameraPos.x = x;
	CameraPos.y = y;
	CameraPos.z = z;
}

void Camera::SetCameraPosition(const glm::vec3& newpos)
{
	CameraPos = newpos;
}

