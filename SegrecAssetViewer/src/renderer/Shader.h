#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>

class Shader
{
public:
	Shader(const char* vertexPath, const char* fragmentPath);
	void Use() const;
	unsigned int GetID() const;

	void SetInt(const char* name, int value);
	void SetMat4(const char* name, glm::mat4& mat);

private:
	unsigned int m_ID;
};

#endif // !SHADER_H
