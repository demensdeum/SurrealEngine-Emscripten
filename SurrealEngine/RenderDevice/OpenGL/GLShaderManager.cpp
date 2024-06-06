#include "GLShaderManager.h"

#include "GLShaderDefinitions.h"
#include <iostream>

GLShaderManager::GLShaderManager()
{
	sceneShader = std::make_unique<GLShader>();

	LoadShaderCode(vertexShaderCode, fragmentShaderCode, 0);
}

void GLShaderManager::LoadShaderCode(const std::string& vertexShaderCode, const std::string& fragmentShaderCode, int debug_index)
{
	std::cout << "LoadShaderCode " << debug_index << std::endl;
	sceneShader->Compile(vertexShaderCode.c_str(), fragmentShaderCode.c_str(), nullptr, 0);

	// Set the common Uniforms
	sceneShader->SetUniformSampler2D("tex", TEXTURE_LAYOUT_LOCATION);
	sceneShader->SetUniformSampler2D("texLightmap", TEXTURE_LIGHTMAP_LAYOUT_LOCATION);
	sceneShader->SetUniformSampler2D("texMacro", TEXTURE_MACRO_LAYOUT_LOCATION);
	sceneShader->SetUniformSampler2D("texDetail", TEXTURE_DETAIL_LAYOUT_LOCATION);
}