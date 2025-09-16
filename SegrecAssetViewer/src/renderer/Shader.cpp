#include "Shader.h"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
	std::string vertexCode, fragmentCode;
	std::ifstream vShaderFile, fShaderFile;

	// exceptions
	vShaderFile.exceptions(std::ifstream::failbit || std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit || std::ifstream::badbit);
	try
	{
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);

		std::stringstream vShaderStream, fShaderStream;

		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();

		vShaderFile.close();
		fShaderFile.close();

		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULY_READ" << std::endl;
	}
	const char* vShaderSource = vertexCode.c_str();
	const char* fShaderSource = fragmentCode.c_str();


	unsigned int vertex, fragment;
	int success;
	char infoLog[512];

	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderSource, 0);
	glCompileShader(vertex);
	
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
		std::cout << "SHADER::VERTEX::COMPILE_FAILED\n" << infoLog << std::endl;
	}

	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderSource, 0);
	glCompileShader(fragment);
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragment, 512, NULL, infoLog);
		std::cout << "SHADER::FRAGMENT::COMPILE_FAILED\n" << infoLog << std::endl;
	}

	m_ID = glCreateProgram();

	glAttachShader(m_ID, vertex);
	glAttachShader(m_ID, fragment);
	glLinkProgram(m_ID);

	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

void Shader::Use() const
{
	glUseProgram(m_ID);
}

unsigned int Shader::GetID() const
{
	return m_ID;
}

void Shader::SetInt(const char* name, int value) const
{
	glUniform1i(glGetUniformLocation(m_ID, name), value);
}

void Shader::SetFloat(const char* name, float value) const
{
	glUniform1f(glGetUniformLocation(m_ID, name), value);
}

void Shader::SetVec3(const char* name, glm::vec3& value) const
{
	glUniform3fv(glGetUniformLocation(m_ID, name), 1, &value[0]);
}

void Shader::SetVec3(const char* name, float x, float y, float z) const
{
	glUniform3f(glGetUniformLocation(m_ID, name), x, y, z);
}

void Shader::SetMat4(const char* name, glm::mat4& mat) const
{
	glUniformMatrix4fv(glGetUniformLocation(m_ID, name), 1, GL_FALSE, &mat[0][0]);
}