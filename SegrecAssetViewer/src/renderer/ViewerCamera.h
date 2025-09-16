#ifndef VIEWERCAMERA_H
#define VIEWERCAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>

enum Input
{
	LEFT_MOUSE_BUTTON,
	RIGHT_MOUSE_BUTTON
};

class ViewerCamera
{
public:
	ViewerCamera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 5.0f),
		         glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
		         glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f),
		         glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f));

	void CursorMovement(double xpos, double ypos, GLboolean constrainPitch = true);
	void ProcessMouseScroll(float yoffset);
	glm::mat4 GetViewMatrix();
	float GetFOV() const;
	glm::vec3 GetPosition() const;
	void SetFocus(bool isFocused);

private:
	void UpdateVectors();

public:
	glm::vec3 m_Position;

private:
	glm::mat4 m_ViewMatrix;

	glm::vec3 m_Front;
	glm::vec3 m_Up;
	glm::vec3 m_Right;
	glm::vec3 m_WorldUp;
	glm::vec3 m_Target;

	float m_FOV = 45.0f;
	float m_Yaw = 90.0f;
	float m_Pitch = 0.0f;
	float m_Radius = 5.0f;
	float m_Sensitivity = 0.1f;

	bool m_IsFocused = false;
};

#endif // !VIEWERCAMERA_H
