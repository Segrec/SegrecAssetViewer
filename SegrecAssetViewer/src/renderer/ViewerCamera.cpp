#include "ViewerCamera.h"

#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

bool firstMouse = true;
float lastX = 0.0f;
float lastY = 0.0f;

ViewerCamera::ViewerCamera(glm::vec3 position, glm::vec3 up, glm::vec3 front, glm::vec3 target)
	: m_Position(position), m_Up(up), m_Front(front), m_WorldUp(up), m_Target(target)
{
	UpdateVectors();
}

void ViewerCamera::CursorMovement(double xpos, double ypos, GLboolean constrainPitch)
{
	if (m_IsFocused)
	{
		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos;
		lastX = xpos;
		lastY = ypos;

		xoffset *= m_Sensitivity;
		yoffset *= m_Sensitivity;

		m_Yaw += xoffset;
		m_Pitch -= yoffset;

		if (constrainPitch)
		{
			if (m_Pitch > 89.0f)
				m_Pitch = 89.0f;
			if (m_Pitch < -89.0f)
				m_Pitch = -89.0f;
		}

		float camX = m_Radius * cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
		float camY = m_Radius * sin(glm::radians(m_Pitch));
		float camZ = m_Radius * sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));

		m_Position = glm::vec3(camX, camY, camZ);

		UpdateVectors();
	}
}

void ViewerCamera::ProcessMouseScroll(float yoffset)
{
	m_Radius -= yoffset;
	if (m_Radius < 1.0f) m_Radius = 1.0f; // minimální vzdálenost
	if (m_Radius > 50.0f) m_Radius = 50.0f; // maximální vzdálenost

	// pøepoèítat novou pozici kamery
	float camX = m_Radius * cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
	float camY = m_Radius * sin(glm::radians(m_Pitch));
	float camZ = m_Radius * sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
	m_Position = glm::vec3(camX, camY, camZ);

	UpdateVectors();
}

glm::mat4 ViewerCamera::GetViewMatrix()
{
	return m_ViewMatrix;
}

float ViewerCamera::GetFOV()
{
	return m_FOV;
}

void ViewerCamera::SetFocus(bool isFocused)
{
	m_IsFocused = isFocused;
	firstMouse = true;
}

void ViewerCamera::UpdateVectors()
{
	glm::vec3 target = m_Target;
	m_Front = glm::normalize(target - m_Position);
	m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
	m_Up = glm::normalize(glm::cross(m_Right, m_Front));
	m_ViewMatrix = glm::lookAt(m_Position, target, m_Up);
}
