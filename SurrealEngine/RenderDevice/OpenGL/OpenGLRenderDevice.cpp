#include "OpenGLRenderDevice.h"
#include "Window/Window.h"

#include "GLTexture.h"

#include <stdexcept>

#include "Window/SDL2/SDL2Window.h"
#include <SDL2/SDL.h>
#include <iostream>

const int framebufferWidth = 1920;
const int framebufferHeight = 1080;

std::map<uint64_t, GLuint> texturesCache;

void clearTexturesCache() {
	for (auto iter = std::cbegin(texturesCache); iter != std::cend(texturesCache); ++iter) {
		glDeleteTextures(1, &iter->second);
	}
	texturesCache.clear();
}

float Aspect = 0.0f;
float RProjZ = 0.0f;
float RFX2 = 0.0f;
float RFY2 = 0.0f;
mat4 objectToProjection;

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
	}

	return 0;
}

std::vector<FColor> P8_Convert(FTextureInfo *info, size_t mipmapLevel)
{
	std::vector<FColor> result;

	UnrealMipmap *mipmap = &info->Texture->Mipmaps[mipmapLevel];
	size_t mipmapWidth = mipmap->Width;
	size_t mipmapHeight = mipmap->Height;

	FColor *palette = info->Palette;

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

OpenGLRenderDevice::OpenGLRenderDevice(GameWindow *InWindow)
{	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

	std::cout << "OpenGLRenderDevice::OpenGLRenderDevice(GameWindow* InWindow)" << std::endl;
	Viewport = InWindow;

	Framebuffers.reset(new GLFrameBufferManager(this));
	Textures.reset(new GLTextureManager());
	Shaders.reset(new GLShaderManager());

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
}

OpenGLRenderDevice::~OpenGLRenderDevice()
{
	clearTexturesCache();
	initializeAndBindFramebufferTexture();

	Textures->ClearTextures();
	Framebuffers.reset();
	Shaders.reset();
}

void OpenGLRenderDevice::Flush(bool AllowPrecache)
{
	// Flush all OpenGL resources
	std::cout << "OpenGLRenderDevice::Flush(bool AllowPrecache)" << std::endl;

	clearTexturesCache();
	clearFbo();
	initializeAndBindFramebufferTexture();
}

bool OpenGLRenderDevice::Exec(std::string Cmd, OutputDevice &Ar)
{
	std::cout << "OpenGLRenderDevice::Exec" << std::endl;
	return false;
}

void OpenGLRenderDevice::Lock(vec4 FlashScale, vec4 FlashFog, vec4 ScreenClear)
{
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
	renderingStartDate = now_ms.time_since_epoch();	

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	std::cout << "OpenGLRenderDevice::Lock(vec4 FlashScale, vec4 FlashFog, vec4 ScreenClear)" << std::endl;
	glEnable(GL_DEPTH_TEST);
	
	glClearColor(0.2f, 0.35f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
}

void OpenGLRenderDevice::bindTexture(FTextureInfo *texture) {
	GLuint textureBinding;

	auto textureWidth = texture->Mips[0].Width;
	auto textureHeight = texture->Mips[0].Height;

	if (texturesCache.find(texture->CacheID) == texturesCache.end()) {
		SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(
			0,
			textureWidth,
			textureHeight,
			32,
			SDL_PIXELFORMAT_RGBA32
		);

		glGenTextures(1, &textureBinding);
		glBindTexture(GL_TEXTURE_2D, textureBinding);
		texturesCache[texture->CacheID] = textureBinding;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		generateMipMap(texture, surface);

		auto palleteMode = GL_RGBA;

		glTexImage2D(
			GL_TEXTURE_2D, 
			0, 
			palleteMode, 
			surface->w, 
			surface->h, 
			0, 
			palleteMode, 
			GL_UNSIGNED_BYTE, 
			surface->pixels
		);
	}
	else {
		textureBinding = texturesCache[texture->CacheID];
		glBindTexture(GL_TEXTURE_2D, textureBinding);
	}
	glActiveTexture(GL_TEXTURE0);	
}

void OpenGLRenderDevice::drawVerticesForTexture(FTextureInfo *Texture, std::vector<Vertex> *verticesVector) {

	GLuint shader_program = Shaders->shaders[DrawComplexSurfaceShader]->ProgramID;
	glUseProgram(shader_program);

	GLint pos = glGetAttribLocation(shader_program, "vertex");		
	GLint uvSlot = glGetAttribLocation(shader_program, "uvIn");
	GLint textureSlot = glGetUniformLocation(shader_program, "texture");

	Vertex *vertices = verticesVector->data();
	GLsizei verticesSize = sizeof(Vertex) * verticesVector->size();
	
	glBufferData(GL_ARRAY_BUFFER, verticesSize, vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glEnableVertexAttribArray(pos);

	auto projectionMatrixUniform = glGetUniformLocation(shader_program, "objectToProjectionMatrix");
	glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, (const GLfloat *)&objectToProjection);

	glVertexAttribPointer(uvSlot, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)(sizeof(Vertex::Position)));
	glEnableVertexAttribArray(uvSlot);

	bindTexture(Texture);

	glUniform1i(textureSlot, 0);

	glDrawArrays(
		GL_TRIANGLE_FAN,
		0,
		verticesVector->size()
	);
}

void OpenGLRenderDevice::Unlock(bool Blit)
{
	std::cout << "OpenGLRenderDevice::Unlock(bool Blit)" << Blit << std::endl;

	if (Blit)
	{
		drawFramebufferTextureOnScreen();
		SDL_GL_SwapWindow(SDL2Window::currentWindow);

    	auto now = std::chrono::system_clock::now();
    	auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
		std::chrono::milliseconds renderingEndDate = now_ms.time_since_epoch();

		std::chrono::milliseconds duration = renderingEndDate - renderingStartDate;
    	double fps = 1000.0 / duration.count();

    	std::cout << "Rendering duration: " << duration.count() << " ms" << std::endl;
    	std::cout << "FPS: " << fps << std::endl;
	}
}

Vertex Vertexxx(GLfloat x, GLfloat y, GLfloat z)
{
	Vertex vertex;
	vertex.Position[0] = x;
	vertex.Position[1] = y;
	vertex.Position[2] = z;

	vertex.TextureUV[0] = x;
	vertex.TextureUV[1] = y;

	return vertex;
}

void OpenGLRenderDevice::generateMipMap(FTextureInfo *Texture, SDL_Surface *surface)
{
	auto textureWidth = Texture->Mips[0].Width;
	auto textureHeight = Texture->Mips[0].Height;	
	auto miplevel = 0;
	{
		UnrealMipmap* mipmap = &Texture->Mips[0];
		uint8_t *mipmapData = mipmap->Data.data();

		if (!mipmap || !mipmapData) {
			std::cout << "mipmap or mipmapData is nullptr" << std::endl;
			return;			
		} 

		GLuint textureFormat = TextureFormatToGL(Texture->Format);

		if (textureFormat >= GL_COMPRESSED_RGBA_S3TC_DXT1_EXT && textureFormat <= GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
		{
			glCompressedTexImage2D(
				GL_TEXTURE_2D, 
				miplevel, 
				textureFormat, 
				mipmap->Width, 
				mipmap->Height, 
				0, 
				0, 
				mipmapData
			);
		}
		else
		{
			if (Texture->Format == TextureFormat::P8)
			{
				// Convert P8 to RGBA32
				auto converted_data = P8_Convert(Texture, miplevel);
				mipmapData = (uint8_t *)converted_data.data();

				int cursor = 0;
				for (auto i = 0; i < textureWidth * textureHeight * 4; i += 4)
				{

					auto surfacePixels = (Uint8 *)surface->pixels;

					auto pixels = mipmapData;

					auto redComponent = pixels[i];
					auto greenComponent = pixels[i + 1];
					auto blueComponent = pixels[i + 2];
					auto alphaComponent = pixels[i + 3];

					surfacePixels[cursor] = redComponent;
					surfacePixels[cursor + 1] = greenComponent;
					surfacePixels[cursor + 2] = blueComponent;
					surfacePixels[cursor + 3] = alphaComponent;


					cursor += 4;
				}
			}
		}
	}	
}

void OpenGLRenderDevice::generateMipMap(FSurfaceInfo &Surface, SDL_Surface *surface)
{
	generateMipMap(Surface.Texture, surface);
}

inline float GetUMult(const FTextureInfo& Info) { return 1.0f / (Info.UScale * Info.USize); }
inline float GetVMult(const FTextureInfo& Info) { return 1.0f / (Info.VScale * Info.VSize); }

void populateVertexBuffer(
	std::vector<Vertex> *verticesVector,
	FTextureInfo& Info,
	const GouraudVertex *Pts, 
	int NumPts
) {
	uint32_t vcount = NumPts;

	float UMult = GetUMult(Info);
	float VMult = GetVMult(Info);

	for (uint32_t i = 0; i < vcount; i++)
	{
		vec3 point = Pts[i].Point;
		float u = Pts[i].UV.s * UMult;
		float v = Pts[i].UV.t * VMult;

		Vertex vertex;
		vertex.Position[0] = point.x;
		vertex.Position[1] = point.y;
		vertex.Position[2] = point.z;
		vertex.TextureUV[0] = u;
		vertex.TextureUV[1] = v;

		verticesVector->push_back(vertex);
	}	
}

void populateVertexVector(
	std::vector<Vertex> *verticesVector,
	FSurfaceFacet &Facet,
	FSurfaceInfo& Surface	
) {
	auto pts = Facet.Vertices;
	uint32_t vcount = Facet.VertexCount;

	float UDot = dot(Facet.MapCoords.XAxis, Facet.MapCoords.Origin);
	float VDot = dot(Facet.MapCoords.YAxis, Facet.MapCoords.Origin);

	float UPan = UDot + Surface.Texture->Pan.x;
	float VPan = VDot + Surface.Texture->Pan.y;
	float UMult = GetUMult(*Surface.Texture);
	float VMult = GetVMult(*Surface.Texture);

	for (uint32_t i = 0; i < vcount; i++)
	{
		vec3 point = pts[i];
		float u = dot(Facet.MapCoords.XAxis, point);
		float v = dot(Facet.MapCoords.YAxis, point);

		Vertex vertex;
		vertex.Position[0] = point.x;
		vertex.Position[1] = point.y;
		vertex.Position[2] = point.z;
		vertex.TextureUV[0] = (u - UPan) * UMult;
		vertex.TextureUV[1] = (v - VPan) * VMult;

		verticesVector->push_back(vertex);
	}	
}

void OpenGLRenderDevice::drawComplexSurfaceToTexture(FSurfaceInfo &Surface, FSurfaceFacet &Facet) {
	std::vector<Vertex> verticesVector;
	populateVertexVector(&verticesVector, Facet, Surface);
	drawVerticesForTexture(Surface.Texture, &verticesVector);	
}

void OpenGLRenderDevice::initializeAndBindFramebufferTexture() {
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glGenTextures(1, &fboTexture);
	glBindTexture(GL_TEXTURE_2D, fboTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, framebufferWidth, framebufferHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenTextures(1, &fboDepthBufferTexture);
	glBindTexture(GL_TEXTURE_2D, fboDepthBufferTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, framebufferWidth, framebufferHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fboDepthBufferTexture, 0);	
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "Error: Framebuffer is not complete!" << std::endl;
		exit(1);
	}
}

void OpenGLRenderDevice::clearFbo() {
	glDeleteBuffers(1, &fbo);
	glDeleteTextures(1, &fboDepthBufferTexture);
	glDeleteTextures(1, &fboTexture);
}

void OpenGLRenderDevice::drawFramebufferTextureOnScreen() {

	GLuint shader_program = Shaders->shaders[DrawComplexSurfaceShader]->ProgramID;
	glUseProgram(shader_program);

	GLint pos = glGetAttribLocation(shader_program, "vertex");		
	GLint uvSlot = glGetAttribLocation(shader_program, "uvIn");
	GLint textureSlot = glGetUniformLocation(shader_program, "texture");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);

	const GLfloat Z_COORD = 0.89f;

	const GLfloat ASPECT_RATIO = 16.0f / 9.0f;

	std::vector<Vertex> verticesVector = {
		{{-0.5f * ASPECT_RATIO,  0.5f, Z_COORD}, {0.0f, 0.0f}},
		{{ 0.5f * ASPECT_RATIO, -0.5f, Z_COORD}, {1.0f, 1.0f}},
		{{-0.5f * ASPECT_RATIO, -0.5f, Z_COORD}, {0.0f, 1.0f}},

		{{-0.5f * ASPECT_RATIO,  0.5f, Z_COORD}, {0.0f, 0.0f}},
		{{ 0.5f * ASPECT_RATIO,  0.5f, Z_COORD}, {1.0f, 0.0f}},
		{{ 0.5f * ASPECT_RATIO, -0.5f, Z_COORD}, {1.0f, 1.0f}}
	};

	Vertex *vertices = verticesVector.data();

	GLsizei verticesSize = sizeof(Vertex) * verticesVector.size();

	glBufferData(GL_ARRAY_BUFFER, verticesSize, vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glEnableVertexAttribArray(pos);

	auto projectionMatrixUniform = glGetUniformLocation(shader_program, "objectToProjectionMatrix");
	glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, (const GLfloat *)&objectToProjection);

	glActiveTexture(GL_TEXTURE0);

	glVertexAttribPointer(uvSlot, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)(sizeof(Vertex::Position)));
	glEnableVertexAttribArray(uvSlot);


	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, fboTexture);

	glUniform1i(textureSlot, 0);

	glDrawArrays(
		GL_TRIANGLE_FAN,
		0,
		verticesVector.size()
	);
}

void OpenGLRenderDevice::DrawComplexSurface(FSceneNode *Frame, FSurfaceInfo &Surface, FSurfaceFacet &Facet)
{
	//std::cout << "DrawComplexSurface(FSceneNode *Frame, FSurfaceInfo &Surface, FSurfaceFacet &Facet)" << std::endl;
	drawComplexSurfaceToTexture(Surface, Facet);
}

void OpenGLRenderDevice::DrawGouraudPolygon(
	FSceneNode *Frame, 
	FTextureInfo &Info, 
	const GouraudVertex *Pts, 
	int NumPts, 
	uint32_t PolyFlags
)
{
	GLuint shader_program = Shaders->shaders[DrawComplexSurfaceShader]->ProgramID;
	glUseProgram(shader_program);

	GLint pos = glGetAttribLocation(shader_program, "vertex");		
	GLint uvSlot = glGetAttribLocation(shader_program, "uvIn");
	GLint textureSlot = glGetUniformLocation(shader_program, "texture");

	std::vector<Vertex> verticesVector;
	populateVertexBuffer(&verticesVector, Info, Pts, NumPts);

	Vertex *vertices = verticesVector.data();

	GLsizei verticesSize = sizeof(Vertex) * verticesVector.size();

	GLuint vbo;

	glBufferData(GL_ARRAY_BUFFER, verticesSize, vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glEnableVertexAttribArray(pos);

	auto projectionMatrixUniform = glGetUniformLocation(shader_program, "objectToProjectionMatrix");
	glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, (const GLfloat *)&objectToProjection);

	glVertexAttribPointer(uvSlot, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)(sizeof(Vertex::Position)));
	glEnableVertexAttribArray(uvSlot);

	bindTexture(&Info);
	glUniform1i(textureSlot, 0);

	glDrawArrays(
		GL_TRIANGLE_FAN,
		0,
		verticesVector.size()
	);
}

void OpenGLRenderDevice::DrawTile(FSceneNode *Frame, FTextureInfo &Info,
								  float X, float Y, float XL, float YL,
								  float U, float V, float UL, float VL, float Z,
								  vec4 Color, vec4 Fog, uint32_t PolyFlags)
{
    glDisable(GL_DEPTH_TEST);
    auto width = Info.Mips[0].Width;
    auto height = Info.Mips[0].Height;

    GLfloat u = GLfloat(U) / width;
    GLfloat v = GLfloat(V) / height;
    GLfloat ul = GLfloat(UL) / width;
    GLfloat vl = GLfloat(VL) / height;

    GLfloat ndcX = (X / framebufferWidth) * 2.0f - 1.0f;
    GLfloat ndcY = (Y / framebufferHeight) * 2.0f - 1.0f; // Invert Y-axis
    GLfloat ndcXL = (XL / framebufferWidth) * 2.0f;
    GLfloat ndcYL = (YL / framebufferHeight) * 2.0f;

    // Adjust the vertex positions for vertical flip and Y-axis inversion
    Vertex vertices[] = {
        {{ndcX, ndcY + ndcYL, Z}, {u, v + vl}}, // bottom-left
        {{ndcX + ndcXL, ndcY + ndcYL, Z}, {u + ul, v + vl}}, // bottom-right
        {{ndcX + ndcXL, ndcY, Z}, {u + ul, v}}, // top-right
        {{ndcX, ndcY, Z}, {u, v}} // top-left
    };
    GLint verticesCount = 4;

    GLuint indices[] = {
        0, 1, 2,
        2, 3, 0};
    GLsizei indicesCount = 6;

    glViewport(0, 0, framebufferWidth, framebufferHeight);

    GLuint vbo, indexBuffer;

    GLsizei verticesSize = sizeof(Vertex) * verticesCount;
    GLsizei indicesSize = sizeof(GLuint) * indicesCount;

    glBufferData(GL_ARRAY_BUFFER, verticesSize, vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize, indices, GL_STATIC_DRAW);

    auto shader_program = Shaders->shaders[DrawTileShader];
    auto shaderProgramID = shader_program->ProgramID;
    glUseProgram(shaderProgramID);
    auto pos = glGetAttribLocation(shaderProgramID, "vertex");

    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glEnableVertexAttribArray(pos);

    GLint uvSlot = glGetAttribLocation(shaderProgramID, "uvIn");
    glVertexAttribPointer(uvSlot, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)(sizeof(Vertex::Position)));
    glEnableVertexAttribArray(uvSlot);

    bindTexture(&Info);

    GLint textureSlot = glGetUniformLocation(shaderProgramID, "texture");
    glUniform1i(textureSlot, 0);

    glDrawElements(
        GL_TRIANGLES,
        indicesCount,
        GL_UNSIGNED_INT,
        0);

    glDeleteBuffers(1, &indexBuffer);
}

void OpenGLRenderDevice::Draw3DLine(FSceneNode *Frame, vec4 Color, vec3 P1, vec3 P2)
{
	std::cout << "OpenGLRenderDevice::Draw3DLine(FSceneNode* Frame, vec4 Color, vec3 P1, vec3 P2)" << std::endl;
}

void OpenGLRenderDevice::Draw2DLine(FSceneNode *Frame, vec4 Color, vec3 P1, vec3 P2)
{
	std::cout << "OpenGLRenderDevice::Draw2DLine" << std::endl;
}

void OpenGLRenderDevice::Draw2DPoint(FSceneNode *Frame, vec4 Color, float X1, float Y1, float X2, float Y2, float Z)
{
	std::cout << "OpenGLRenderDevice::Draw2DPoint" << std::endl;
}

void OpenGLRenderDevice::ClearZ(FSceneNode *Frame)
{
	std::cout << "OpenGLRenderDevice::ClearZ" << std::endl;
	glClear(GL_DEPTH_BUFFER_BIT);	
}

void OpenGLRenderDevice::ReadPixels(FColor *Pixels)
{
	// MEMORY LEAK?
	return;
	std::cout << "OpenGLRenderDevice::ReadPixels(FColor* Pixels)" << std::endl;
	auto readPixels = Framebuffers->SceneFrameBuffer->ReadPixelData();

	memcpy(Pixels, readPixels.data(), readPixels.size());
}

void OpenGLRenderDevice::EndFlash()
{
	// std::cout << "OpenGLRenderDevice::EndFlash()" << std::endl;
}

void OpenGLRenderDevice::SetSceneNode(FSceneNode *Frame)
{
	//std::cout << "SetSceneNode(FSceneNode *Frame)" << std::endl;
	CurrentFrame = Frame;
	Aspect = Frame->FY / Frame->FX;
	RProjZ = (float)std::tan(radians(Frame->FovAngle) * 0.5);
	RFX2 = 2.0f * RProjZ / Frame->FX;
	RFY2 = 2.0f * RProjZ * Aspect / Frame->FY;

	glViewport((float)Frame->XB, (float)Frame->YB, (float)Frame->X, (float)Frame->Y);

	objectToProjection = mat4::frustum(-RProjZ, RProjZ, -Aspect * RProjZ, Aspect * RProjZ, 1.0f, 32768.0f, handedness::left, clipzrange::zero_positive_w);
	objectToProjection = objectToProjection * Frame->WorldToView * Frame->ObjectToWorld;
}

void OpenGLRenderDevice::PrecacheTexture(FTextureInfo &Info, uint32_t PolyFlags)
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

void OpenGLRenderDevice::UpdateTextureRect(FTextureInfo &Info, int U, int V, int UL, int VL)
{
	std::cout << "OpenGLRenderDevice::UpdateTextureRect" << std::endl;
}
