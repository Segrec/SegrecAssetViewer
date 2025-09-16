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

	void SetInt(const char* name, int value) const;
	void SetFloat(const char* name, float value) const;
	void SetVec3(const char* name, glm::vec3& value) const;
	void SetVec3(const char* name, float x, float y, float z) const;
	void SetMat4(const char* name, glm::mat4& mat) const;

private:
	unsigned int m_ID;
};

#endif // !SHADER_H
