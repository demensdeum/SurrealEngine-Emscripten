#include "GLTextureManager.h"

GLTextureManager::~GLTextureManager()
{
	ClearTextures();
}

void GLTextureManager::ClearTextures()
{
	Textures.clear();
}

GLTexture *GLTextureManager::GetTexture(FTextureInfo* info)
{
	GLTexture *texture = nullptr;
	if (auto result = Textures.find(info); result != Textures.end())	{
		texture = Textures[info];
	}

	if (!texture)
	{
		auto newTex = new GLTexture();
		newTex->Generate(info);

		Textures[info] = newTex;

		return newTex;
	}

	return texture;
}