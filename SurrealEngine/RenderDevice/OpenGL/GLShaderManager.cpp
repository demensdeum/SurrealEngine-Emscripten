#include "GLShaderManager.h"

#include <iostream>

#include "Shaders/GLDrawTileShaders.h"
#include "Shaders/GLDrawComplexSurfaceShaders.h"

GLShaderManager::GLShaderManager()
{
	LoadShaderCode(DrawComplexSurfaceShader, drawTileVertexShaderCode, drawTileFragmentShaderCode, 0);
	LoadShaderCode(DrawTileShader, drawTileVertexShaderCode, drawTileFragmentShaderCode, 0);
}

GLShaderManager::~GLShaderManager()
{
	flush();
}

void GLShaderManager::LoadShaderCode(
	GLShaderID shaderID,
	const std::string& vertexShaderCode,
	const std::string& fragmentShaderCode, 
	int debug_index
)
{
	std::cout << "LoadShaderCode " << debug_index << std::endl;

	auto shader = new GLShader();
	shader->Compile(vertexShaderCode, fragmentShaderCode, "", 0);

	shaders[shaderID] = shader;
}

void GLShaderManager::flush()
{
	for (const auto& pair : shaders) {
		delete pair.second;
	}
}