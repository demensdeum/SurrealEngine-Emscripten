#include "GLTexture.h"

GLTexture::GLTexture()
{
	glGenTextures(1, &textureID);
}

GLTexture::~GLTexture()
{
	glDeleteTextures(1, &textureID);
}

void GLTexture::Bind()
{
	glBindTexture(GL_TEXTURE_2D, textureID);
}

void GLTexture::Unbind()
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

void GLTexture::Generate(FTextureInfo* info)
{
	size_t numMips = info->Texture->Mipmaps.size();
	GLuint textureFormat = TextureFormatToGL(info->Format);

	Bind();

	for (size_t miplevel = 0; miplevel < numMips; miplevel++)
	{
		auto& mipmap = info->Texture->Mipmaps[miplevel];

		uint8_t* mipmapData = mipmap.Data.data();

		if (textureFormat >= GL_COMPRESSED_RGBA_S3TC_DXT1_EXT && textureFormat <= GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
		{
			glCompressedTexImage2D(GL_TEXTURE_2D, miplevel, textureFormat, mipmap.Width, mipmap.Height, 0, 0, mipmapData);
		}
		else
		{
			if (info->Format == TextureFormat::P8)
			{
				// Convert P8 to RGBA32
				auto converted_data = P8_Convert(info, miplevel);
				mipmapData = (uint8_t*)converted_data.data();
			}
			
			glTexImage2D(GL_TEXTURE_2D, miplevel, textureFormat, mipmap.Width, mipmap.Height, 0, textureFormat, GL_UNSIGNED_BYTE, mipmapData);
		}
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	Unbind();
}

GLuint GLTexture::TextureFormatToGL(TextureFormat format)
{
	switch (format)
	{
	// P8 is the Eight-bit Palettized format where the actual texture is 32 bit RGBA
	// but it is stored as indexes of a palette instead, saving on disk space 
	case TextureFormat::P8:
		return GL_RGBA32I;

	// Compressed texture formats DXT1/3/5
	// An OpenGL 3.3 capable card should already support these at this point.
	case TextureFormat::BC1:
		return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
	case TextureFormat::BC2:
		return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
	case TextureFormat::BC3:
		return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;

	case TextureFormat::R1:
		return 0; // No GL equivalent

	case TextureFormat::R8:
		return GL_R8;

	case TextureFormat::R16:
		return GL_R16;
	case TextureFormat::R16_F:
		return GL_R16F;
	case TextureFormat::R16_I:
		return GL_R16I;
	case TextureFormat::R16_S:
		return GL_R16_SNORM;
	case TextureFormat::R16_UI:
		return GL_R16UI;

	case TextureFormat::R32:
	case TextureFormat::R32_I:
	case TextureFormat::R32_S:
		return GL_R32I;
	case TextureFormat::R32_F:
		return GL_R32F;
	case TextureFormat::R32_UI:
		return GL_R32UI;

	case TextureFormat::BGR8:
		return GL_BGR;

	case TextureFormat::RGB16_:
		return GL_RGB16;
	case TextureFormat::RGB16_F:
		return GL_RGB16F;
	case TextureFormat::RGB16_I:
		return GL_RGB16I;
	case TextureFormat::RGB16_S:
		return GL_RGB16_SNORM;
	case TextureFormat::RGB16_UI:
		return GL_RGB16UI;

	case TextureFormat::RGBA8_:
		return GL_RGBA8;
	case TextureFormat::RGBA8_S:
		return GL_RGBA8_SNORM;
	case TextureFormat::RGBA8_I:
		return GL_RGBA8I;
	case TextureFormat::RGBA8_UI:
		return GL_RGBA8UI;

	case TextureFormat::RGB9E5:
		return GL_RGB9_E5;
	
	case TextureFormat::RGB10A2:
	case TextureFormat::RGB10A2_I:
	case TextureFormat::RGB10A2_LM:
	case TextureFormat::RGB10A2_S:
		return GL_RGB10_A2; // No GL equivalent for the variants???
	case TextureFormat::RGB10A2_UI:
		return GL_RGB10_A2UI;

	case TextureFormat::RGBA16:
		return GL_RGBA16;
	case TextureFormat::RGBA16_S:
		return GL_RGBA16_SNORM;
	case TextureFormat::RGBA16_F:
		return GL_RGBA16F;
	case TextureFormat::RGBA16_UI:
		return GL_RGBA16UI;
	case TextureFormat::RGBA16_I:
		return GL_RGBA16I;

	case TextureFormat::RGBA32:
	case TextureFormat::RGBA32_I:
		return GL_RGBA32I;
	case TextureFormat::RGBA32_F:
		return GL_RGBA32F;
	case TextureFormat::RGBA32_UI:
		return GL_RGBA32UI;

	case TextureFormat::RGBA64_F:
		return 0; // No GL equivalent

	case TextureFormat::ARGB8:
	case TextureFormat::ABGR8:
		return 0; // No GL equivalent

	case TextureFormat::BGRA8_LM:
	case TextureFormat::BGRA8:
		return GL_BGRA;

	case TextureFormat::B5G6R5:
		return 0; // GL has GL_RGB565, which is in different order
	case TextureFormat::R5G6B5:
		return GL_RGB565;
	
	case TextureFormat::RG16:
		return GL_RG16;
	case TextureFormat::RG16_F:
		return GL_RG16F;
	case TextureFormat::RG16_S:
		return GL_RG16_SNORM;
	case TextureFormat::RG16_I:
		return GL_RG16I;
	case TextureFormat::RG16_UI:
		return GL_RG16UI;

	case TextureFormat::RG32:
	case TextureFormat::RG32_I:
	case TextureFormat::RG32_S:
		return GL_RG32I;
	case TextureFormat::RG32_F:
		return GL_RG32F;
	case TextureFormat::RG32_UI:
		return GL_RG32UI;
	
	case TextureFormat::R11G11B10_F:
		return GL_R11F_G11F_B10F;

	// ASTC formats seem to be an OpenGL ES 3.0+ and OpenGL 4.3+ thing...
	/*
	case TextureFormat::ASTC_4x4:
		return GL_COMPRESSED_RGBA_ASTC_4x4_KHR;
	case TextureFormat::ASTC_5x4:
		return GL_COMPRESSED_RGBA_ASTC_5x4_KHR;
	case TextureFormat::ASTC_5x5:
		return GL_COMPRESSED_RGBA_ASTC_5x5_KHR;
	case TextureFormat::ASTC_6x5:
		return GL_COMPRESSED_RGBA_ASTC_6x5_KHR;
	case TextureFormat::ASTC_6x6:
		return GL_COMPRESSED_RGBA_ASTC_6x6_KHR;
	*/
	}

	return 0;
}

std::vector<FColor> GLTexture::P8_Convert(FTextureInfo* info, size_t mipmapLevel)
{
	std::vector<FColor> result;

	UnrealMipmap* mipmap = &info->Texture->Mipmaps[mipmapLevel];
	size_t mipmapWidth = mipmap->Width;
	size_t mipmapHeight = mipmap->Height;

	FColor* palette = info->Palette;

	result.resize(mipmapWidth * mipmapHeight);

	if (info->Texture->bMasked())
	{
		FColor transparent(0, 0, 0, 0);

		for (size_t y = 0; y < mipmapHeight; y++)
		{
			for (size_t x = 0; x < mipmapWidth; x++)
			{
				uint8_t index = mipmap->Data[x + y * mipmapWidth];
				result[x + y * mipmapWidth] = index == 0 ? transparent : palette[index];
			}
		}
	}
	else
	{
		for (size_t y = 0; y < mipmapHeight; y++)
		{
			for (size_t x = 0; x < mipmapWidth; x++)
			{
				uint8_t index = mipmap->Data[x + y * mipmapWidth];
				result[x + y * mipmapWidth] = palette[index];
			}
		}
	}

	return result;
}