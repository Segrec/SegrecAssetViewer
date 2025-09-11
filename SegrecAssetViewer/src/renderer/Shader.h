#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

class Shader
{
public:
	Shader(const char* vertexPath, const char* fragmentPath);
	void Use() const;
	unsigned int GetID() const;

private:
	unsigned int m_ID;
};

#endif // !SHADER_H
