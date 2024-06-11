#pragma once

#include "GLShader.h"

#include <memory>
#include <string>

#include <unordered_map>

enum GLShaderID {
	DrawComplexSurfaceShader,
	DrawTileShader
};

class GLShaderManager
{
public:
	GLShaderManager();
	~GLShaderManager();

	std::unordered_map<GLShaderID, GLShader*> shaders;

private:
	void LoadShaderCode(
		GLShaderID shaderID,
		const std::string& vertexShaderCode,
		const std::string& fragmentShaderCode, 
		int debug_index
	);

	void flush();
};