#pragma once

#include "RenderDevice/RenderDevice.h"
#include <GL/glew.h>

// An OpenGL representation of an UTexture
class GLTexture
{
public:
	GLTexture();
	~GLTexture();

	void Bind();
	void Unbind();

	void Generate(FTextureInfo* info);

	static GLuint TextureFormatToGL(TextureFormat format);
private:
	std::vector<FColor> P8_Convert(FTextureInfo* info, size_t mipmapLevel);
	GLuint textureID;
};