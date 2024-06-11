#include "GLShader.h"

#include <stdexcept>
#include <iostream>

GLShader::GLShader() : ProgramID(0)
{
}

GLShader::~GLShader()
{
	glDeleteProgram(ProgramID);
}

void GLShader::Bind()
{
	glUseProgram(ProgramID);
}

void GLShader::Unbind()
{
	glUseProgram(0);
}

void GLShader::Compile(
		const std::string vertexCode, 
		const std::string fragmentCode, 
		const std::string geometryCode, 
		int debug_index
	)
{
	std::cout << "GLShader::Compile " << debug_index << std::endl;
	GLuint VertexShaderID, FragmentShaderID, GeometryShaderID;

	std::cout << "69GLShader::Compile " << debug_index << std::endl;
	VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	std::cout << "1GLShader::Compile " << debug_index << std::endl;
	auto vc = vertexCode.c_str();
	glShaderSource(VertexShaderID, 1, &vc, NULL);
	std::cout << "2GLShader::Compile " << debug_index << std::endl;
	glCompileShader(VertexShaderID);
	std::cout << "3GLShader::Compile " << debug_index << std::endl;
	CheckCompileErrors(VertexShaderID, "Vertex", 1);

	FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	auto fc = fragmentCode.c_str();
	glShaderSource(FragmentShaderID, 1, &fc, NULL);
	glCompileShader(FragmentShaderID);
	CheckCompileErrors(FragmentShaderID, "Fragment", 2);

	if (!geometryCode.empty())
	{
		GeometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);
		auto gc = geometryCode.c_str();
		glShaderSource(GeometryShaderID, 1, &gc, NULL);
		CheckCompileErrors(GeometryShaderID, "Geometry", 3);
	}

	ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	if (!geometryCode.empty())
		glAttachShader(ProgramID, GeometryShaderID);
	glLinkProgram(ProgramID);
	CheckLinkErrors();

	// Shaders can be freely deleted now
	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);
	if (!geometryCode.empty())
		glDeleteShader(GeometryShaderID);
}

void GLShader::SetUniformMat4(const std::string& uniformName, mat4 value)
{
	GLint uniformLocation = glGetUniformLocation(ProgramID, uniformName.c_str());

	if (uniformLocation == -1)
		throw std::runtime_error(uniformName + " is not a valid Matrix4x4 uniform for the shader!");

	glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, value.matrix);
}

void GLShader::SetUniformSampler2D(const std::string& uniformName, const int GLTextureSlot)
{
	GLint uniformLocation = glGetUniformLocation(ProgramID, uniformName.c_str());

	if (uniformLocation == -1)
		throw std::runtime_error(uniformName + " is not a valid Sampler2D uniform for the shader!");

	glUniform1i(uniformLocation, GLTextureSlot);
}

void GLShader::CheckCompileErrors(GLuint Object, const std::string ObjectType, int debug_index) const
{
	std::cout << "CheckCompileErrors Debug Index: " << debug_index << std::endl;

	GLint success;
	GLchar infoLog[1024];

	glGetShaderiv(Object, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(Object, 1024, NULL, infoLog);
		std::cout << "Error while compiling: " << ObjectType << " shader: " << infoLog << std::endl;

		throw std::runtime_error("Error while compiling " + ObjectType + " shader: " + infoLog);
	}
	else {
		std::cout << "Success shader compile" << std::endl;
	}
}

void GLShader::CheckLinkErrors() const
{
	GLint success;
	GLchar infoLog[1024];

	glGetProgramiv(ProgramID, GL_LINK_STATUS, &success);

	if (!success)
	{
		glGetProgramInfoLog(ProgramID, 1024, NULL, infoLog);

		throw std::runtime_error("Error while linking shader: " + std::string(infoLog));
	}
}