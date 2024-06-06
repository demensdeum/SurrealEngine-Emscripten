#include "GLTextureManager.h"

GLTextureManager::~GLTextureManager()
{
	ClearTextures();
}

void GLTextureManager::ClearTextures()
{
	Textures.clear();
}

GLTexture GLTextureManager::GetTexture(FTextureInfo* info)
{
	GLTexture* texture = &Textures[info];

	if (!texture)
	{
		GLTexture newTex;
		newTex.Generate(info);

		Textures[info] = newTex;

		return newTex;
	}

	return *texture;
}