#pragma once

#include "Math/mat.h"
#include <GL/glew.h>

// Since OpenGL 3.3 doesn't have layout(binding = x) support, we'll bind the samplers the old way.
constexpr int TEXTURE_LAYOUT_LOCATION = 0;
constexpr int TEXTURE_LIGHTMAP_LAYOUT_LOCATION = 1;
constexpr int TEXTURE_MACRO_LAYOUT_LOCATION = 2;
constexpr int TEXTURE_DETAIL_LAYOUT_LOCATION = 3;

class GLShader
{
public:
	GLShader();
	~GLShader();

	void Bind();
	void Unbind();

	void Compile(const char* vertexCode, const char* fragmentCode, const char* geometryCode, int debug_index);

	void SetUniformMat4(const std::string& uniformName, mat4 value);
	void SetUniformSampler2D(const std::string& uniformName, const int GLTextureSlot);

	GLuint ProgramID;
	
private:
	void CheckCompileErrors(GLuint Object, const std::string ObjectType, int debug_index) const;
	void CheckLinkErrors() const;

};