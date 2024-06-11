#include "OpenGLRenderDevice.h"
#include "Window/Window.h"

#include "GLTexture.h"

#include <stdexcept>

#include "Window/SDL2/SDL2Window.h"
#include <SDL2/SDL.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

auto referenceSurface = SDL_LoadBMP("test_texture.bmp"); // must be 24-bit color pallete


SDL_Surface* duplicateSurface(SDL_Surface* original) {
    if (!original) {
        fprintf(stderr, "Original surface is NULL!\n");
        return NULL;
    }

    SDL_Surface* duplicate = SDL_CreateRGBSurfaceWithFormat(0, 
                                                            original->w, 
                                                            original->h, 
                                                            original->format->BitsPerPixel, 
                                                            original->format->format);
    if (!duplicate) {
        fprintf(stderr, "SDL_CreateRGBSurfaceWithFormat Error: %s\n", SDL_GetError());
        return NULL;
    }

    if (SDL_BlitSurface(original, NULL, duplicate, NULL) != 0) {
        fprintf(stderr, "SDL_BlitSurface Error: %s\n", SDL_GetError());
        SDL_FreeSurface(duplicate);
        return NULL;
    }

    return duplicate;
}

GLfloat xOffset = -0.8f;

typedef struct {
    GLfloat Position[3];
	GLfloat TextureUV[2];
} Vertex;

GLuint TextureFormatToGL(TextureFormat format)
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

std::vector<FColor> P8_Convert(FTextureInfo* info, size_t mipmapLevel)
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

OpenGLRenderDevice::OpenGLRenderDevice(GameWindow* InWindow)
{
	std::cout << "OpenGLRenderDevice::OpenGLRenderDevice(GameWindow* InWindow)" << std::endl;
	Viewport = InWindow;

	Framebuffers.reset(new GLFrameBufferManager(this));
	Textures.reset(new GLTextureManager());
	Shaders.reset(new GLShaderManager());
}

OpenGLRenderDevice::~OpenGLRenderDevice()
{
	std::cout << "OpenGLRenderDevice::~OpenGLRenderDevice()" << std::endl;	
	Textures->ClearTextures();
	Framebuffers.reset();
	Shaders.reset();
}

void OpenGLRenderDevice::Flush(bool AllowPrecache)
{
	// Flush all OpenGL resources
	std::cout << "OpenGLRenderDevice::Flush(bool AllowPrecache)" << std::endl;
}

bool OpenGLRenderDevice::Exec(std::string Cmd, OutputDevice& Ar)
{
	std::cout << "OpenGLRenderDevice::Exec" << std::endl;	
	return false;
}

void OpenGLRenderDevice::Lock(vec4 FlashScale, vec4 FlashFog, vec4 ScreenClear)
{
	std::cout << "OpenGLRenderDevice::Lock(vec4 FlashScale, vec4 FlashFog, vec4 ScreenClear)" << std::endl;	
    glClearColor(0.2f, 0.35f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	xOffset -= 0.1;
}

void OpenGLRenderDevice::Unlock(bool Blit)
{
	std::cout << "OpenGLRenderDevice::Unlock(bool Blit)" << std::endl;	

	if (Blit) {
		SDL_GL_SwapWindow(SDL2Window::currentWindow);	
	}
}

Vertex Vertexxx(GLfloat x, GLfloat y, GLfloat z)
{
	Vertex vertex;
	vertex.Position[0] = x;
	vertex.Position[1] = y;
	vertex.Position[2] = z;

	return vertex;
}

void OpenGLRenderDevice::DrawComplexSurface(FSceneNode* Frame, FSurfaceInfo& Surface, FSurfaceFacet& Facet)
{
	std::cout << "OpenGLRenderDevice::DrawComplexSurface(FSceneNode* Frame, FSurfaceInfo& Surface, FSurfaceFacet& Facet)" << std::endl;
	std::cout << "FSurfaceFacet facet vertex count: " << Facet.VertexCount << std::endl;

	auto pts = Facet.Vertices;
	uint32_t vcount = Facet.VertexCount;

	std::vector<Vertex> verticesVector;
	std::vector<GLuint> indicesVector;

	for (uint32_t i = 0; i < vcount; i++)
	{
		vec3 point = pts[i];
		float u = dot(Facet.MapCoords.XAxis, point);
		float v = dot(Facet.MapCoords.YAxis, point);

		Vertex vertex;
		vertex.Position[0] = std::min(std::abs(point.x / 2000.f), 1.f);
		vertex.Position[1] = std::min(std::abs(point.y / 2000.f), 1.f);
		//vertex.Position[2] = point.z / 5000.f;
		vertex.Position[2] = 0;
		vertex.TextureUV[0] = u;
		vertex.TextureUV[1] = v;

		verticesVector.push_back(vertex);
		indicesVector.push_back(i);
	}

	auto surface = duplicateSurface(referenceSurface);

	// verticesVector.clear();
	// verticesVector.push_back(Vertexxx(-1, -1, 0));
	// verticesVector.push_back(Vertexxx(0, 1, 0));
	// verticesVector.push_back(Vertexxx(1, -1, 0));

	// indicesVector.clear();
	// indicesVector.push_back(0);
	// indicesVector.push_back(1);
	// indicesVector.push_back(2);

    Vertex *vertices = verticesVector.data();
    GLuint *indices = indicesVector.data();

	GLsizei verticesSize = sizeof(Vertex) * verticesVector.size();
	GLsizei indicesSize = sizeof(GLuint) * indicesVector.size(); 
	GLsizei indicesCount = indicesVector.size();

	GLuint shader_program = Shaders->shaders[DrawComplexSurfaceShader]->ProgramID;
	GLint pos = glGetAttribLocation(shader_program, "vertex");

    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

	glViewport(0, 0, 1920, 1080);

	GLuint vbo, indexBuffer;

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verticesSize, vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize, indices, GL_STATIC_DRAW);

    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glEnableVertexAttribArray(pos);

    glUseProgram(shader_program);

	glActiveTexture(GL_TEXTURE0);

    GLint uvSlot = glGetAttribLocation(shader_program, "uvIn");
    glVertexAttribPointer(uvSlot, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) (sizeof(Vertex::Position)));
    glEnableVertexAttribArray(uvSlot);

	if (surface == nullptr) {
		std::cout << "CANT LOAD TEXT_TEXTURE!!!" << std::endl;
		exit(1);
	}

    auto surfaceLength = surface->w * surface->h * 3;

    // swap bgr -> rgb

    for (auto i = 0; i < surfaceLength; i += 3) {

        auto pixels = (Uint8 *) surface->pixels;

        auto blueComponent = pixels[i];
        auto greenComponent = pixels[i + 1];
        auto redComponent = pixels[i + 2];

        pixels[i] = redComponent;
        pixels[i + 1] = greenComponent;
        pixels[i + 2] = blueComponent;

    }

    auto palleteMode = GL_RGB;

    GLuint textureBinding;
    glGenTextures(1, &textureBinding);
    glBindTexture(GL_TEXTURE_2D, textureBinding);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, palleteMode, surface->w, surface->h, 0, palleteMode, GL_UNSIGNED_BYTE, surface->pixels);
    
	glActiveTexture(GL_TEXTURE0);

    SDL_FreeSurface(surface);
    //glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
    GLint textureSlot = glGetUniformLocation(shader_program, "texture");
    glUniform1i(textureSlot, 0);

    glDrawElements(
		GL_TRIANGLES, 
		indicesCount,
        GL_UNSIGNED_INT, 
		0
	);

    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &indexBuffer);
    glDeleteTextures(1, &textureBinding);	
}

void OpenGLRenderDevice::DrawGouraudPolygon(FSceneNode* Frame, FTextureInfo& Info, const GouraudVertex* Pts, int NumPts, uint32_t PolyFlags)
{
	//std::cout << "OpenGLRenderDevice::DrawGouraudPolygon(FSceneNode* Frame, FTextureInfo& Info, const GouraudVertex* Pts, int NumPts, uint32_t PolyFlags)" << std::endl;	
}

void OpenGLRenderDevice::DrawTile(FSceneNode* Frame, FTextureInfo& Info,
			  float X, float Y, float XL, float YL, 
			  float U, float V, float UL, float VL, float Z, 
			  vec4 Color, vec4 Fog, uint32_t PolyFlags)
{
	auto surface = duplicateSurface(referenceSurface);

	GLfloat u = GLfloat(U) / 256.f;
	GLfloat v = GLfloat(V) / 256.f;
	GLfloat ul = GLfloat(UL) / 256.f;
	GLfloat vl = GLfloat(VL) / 256.f;

    GLfloat viewportWidth = 1920;
    GLfloat viewportHeight = 1080;

    GLfloat ndcX = (X / viewportWidth) * 2.0f - 1.0f;
    GLfloat ndcY = 1.0f - (Y / viewportHeight) * 2.0f;
    GLfloat ndcXL = (XL / viewportWidth) * 2.0f;
    GLfloat ndcYL = (YL / viewportHeight) * 2.0f;

    Vertex vertices[] = {
        {{ndcX, ndcY, Z},          {u, v}},
        {{ndcX + ndcXL, ndcY, Z},  {u + ul, v}},
        {{ndcX + ndcXL, ndcY - ndcYL, Z}, {u + ul, v + vl}},
        {{ndcX, ndcY - ndcYL, Z},  {u, v + vl}}
    };
	GLint verticesCount = 4;

    GLuint indices[] = {
        0, 1, 2,
        2, 3, 0
    };
	GLsizei indicesCount = 6;

	auto shader_program = Shaders->shaders[DrawTileShader];
	auto shaderProgramID = shader_program->ProgramID;
	GLint pos = glGetAttribLocation(shaderProgramID, "vertex");

    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

	glViewport(0, 0, 1920, 1080);

	GLuint vbo, indexBuffer;

	GLsizei verticesSize = sizeof(Vertex) * verticesCount;
	GLsizei indicesSize = sizeof(GLuint) * indicesCount;

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verticesSize, vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize, indices, GL_STATIC_DRAW);

    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glEnableVertexAttribArray(pos);

    glUseProgram(shaderProgramID);

	glActiveTexture(GL_TEXTURE0);

    GLint uvSlot = glGetAttribLocation(shaderProgramID, "uvIn");
    glVertexAttribPointer(uvSlot, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) (sizeof(Vertex::Position)));
    glEnableVertexAttribArray(uvSlot);

	// auto texture = Textures->GetTexture(&Info);	
	// texture->Bind();
    // GLint textureSlot = glGetUniformLocation(shader_program, "texture");
    // glUniform1i(textureSlot, 0);

	if (surface == nullptr) {
		std::cout << "CANT LOAD TEXT_TEXTURE!!!" << std::endl;
		exit(1);
	}

    auto surfaceLength = surface->w * surface->h * 3;

    // swap bgr -> rgb

    for (auto i = 0; i < surfaceLength; i += 3) {

        auto pixels = (Uint8 *) surface->pixels;

        auto blueComponent = pixels[i];
        auto greenComponent = pixels[i + 1];
        auto redComponent = pixels[i + 2];

        pixels[i] = redComponent;
        pixels[i + 1] = greenComponent;
        pixels[i + 2] = blueComponent;

    }

    auto palleteMode = GL_RGB;

    GLuint textureBinding;
    glGenTextures(1, &textureBinding);
    glBindTexture(GL_TEXTURE_2D, textureBinding);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	auto info = &Info;

	size_t numMips = info->Texture->Mipmaps.size();
	GLuint textureFormat = TextureFormatToGL(info->Format);

	//for (size_t miplevel = 0; miplevel < numMips; miplevel++)
	auto miplevel = 0;
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

				int cursor = 0;
				for (auto i = 0; i < mipmap.Width * mipmap.Height * 4; i += 4) {

					auto surfacePixels = (Uint8 *) surface->pixels;

					auto pixels = mipmapData;

					
					auto redComponent = pixels[i];
					auto greenComponent = pixels[i + 1];
					auto blueComponent = pixels[i + 2];

					surfacePixels[cursor] = redComponent;
					surfacePixels[cursor + 1] = greenComponent;
					surfacePixels[cursor + 2] = blueComponent;

					cursor += 3;
				}			

			}
			
			//glTexImage2D(GL_TEXTURE_2D, miplevel, textureFormat, mipmap.Width, mipmap.Height, 0, textureFormat, GL_UNSIGNED_BYTE, mipmapData);
		}
	}
	glTexImage2D(GL_TEXTURE_2D, 0, palleteMode, surface->w, surface->h, 0, palleteMode, GL_UNSIGNED_BYTE, surface->pixels);


    // BMP -> glTexImage2D(GL_TEXTURE_2D, 0, palleteMode, surface->w, surface->h, 0, palleteMode, GL_UNSIGNED_BYTE, surface->pixels);
    
	glActiveTexture(GL_TEXTURE0);

    SDL_FreeSurface(surface);
    //glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
    GLint textureSlot = glGetUniformLocation(shaderProgramID, "texture");
    glUniform1i(textureSlot, 0);

    glDrawElements(
		GL_TRIANGLES, 
		indicesCount,
        GL_UNSIGNED_INT, 
		0
	);

    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &indexBuffer);
    glDeleteTextures(1, &textureBinding);
}

void OpenGLRenderDevice::Draw3DLine(FSceneNode* Frame, vec4 Color, vec3 P1, vec3 P2)
{
	std::cout << "OpenGLRenderDevice::Draw3DLine(FSceneNode* Frame, vec4 Color, vec3 P1, vec3 P2)" << std::endl;
}

void OpenGLRenderDevice::Draw2DLine(FSceneNode* Frame, vec4 Color, vec3 P1, vec3 P2)
{
	std::cout << "OpenGLRenderDevice::Draw2DLine" << std::endl;
}

void OpenGLRenderDevice::Draw2DPoint(FSceneNode* Frame, vec4 Color, float X1, float Y1, float X2, float Y2, float Z)
{
	std::cout << "OpenGLRenderDevice::Draw2DPoint" << std::endl;
}

void OpenGLRenderDevice::ClearZ(FSceneNode* Frame)
{
	std::cout << "OpenGLRenderDevice::ClearZ" << std::endl;
	glClear(GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderDevice::ReadPixels(FColor* Pixels)
{
	std::cout << "OpenGLRenderDevice::ReadPixels(FColor* Pixels)" << std::endl;
	auto readPixels = Framebuffers->SceneFrameBuffer->ReadPixelData();

	memcpy(Pixels, readPixels.data(), readPixels.size());
}

void OpenGLRenderDevice::EndFlash()
{
	std::cout << "OpenGLRenderDevice::EndFlash()" << std::endl;
}

void OpenGLRenderDevice::SetSceneNode(FSceneNode* Frame)
{
	std::cout << "OpenGLRenderDevice::SetSceneNode" << std::endl;
	//DrawScene();

	CurrentFrame = Frame;

	Aspect = Frame->FY / Frame->FX;

	//glViewport(Frame->X, Frame->Y, Frame->XB, Frame->YB);
}

void OpenGLRenderDevice::PrecacheTexture(FTextureInfo& Info, uint32_t PolyFlags)
{
	std::cout << "OpenGLRenderDevice::PrecacheTexture" << std::endl;
	// TODO: Handle PolyFlags too
	Textures->GetTexture(&Info);
}

bool OpenGLRenderDevice::SupportsTextureFormat(TextureFormat Format)
{
	std::cout << "OpenGLRenderDevice::SupportsTextureFormat" << std::endl;
	return GLTexture::TextureFormatToGL(Format);
}

void OpenGLRenderDevice::UpdateTextureRect(FTextureInfo& Info, int U, int V, int UL, int VL)
{
	std::cout << "OpenGLRenderDevice::UpdateTextureRect" << std::endl;
}
