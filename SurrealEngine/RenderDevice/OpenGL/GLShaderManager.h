#pragma once

#include "GLShader.h"

#include <memory>
#include <string>

class GLShaderManager
{
public:
	GLShaderManager();

	std::unique_ptr<GLShader> sceneShader;

	void LoadShaderCode(const std::string& vertexShaderCode, const std::string& fragmentShaderCode, int debug_index);
};