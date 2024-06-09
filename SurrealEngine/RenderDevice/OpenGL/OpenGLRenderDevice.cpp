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

float tileCount = 0;

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
	tileCount = 0;
}

void OpenGLRenderDevice::Unlock(bool Blit)
{
	std::cout << "OpenGLRenderDevice::Unlock(bool Blit)" << std::endl;	

	if (Blit) {
		SDL_GL_SwapWindow(SDL2Window::currentWindow);	
	}
}

void OpenGLRenderDevice::DrawComplexSurface(FSceneNode* Frame, FSurfaceInfo& Surface, FSurfaceFacet& Facet)
{
	std::cout << "OpenGLRenderDevice::DrawComplexSurface(FSceneNode* Frame, FSurfaceInfo& Surface, FSurfaceFacet& Facet)" << std::endl;
}

void OpenGLRenderDevice::DrawGouraudPolygon(FSceneNode* Frame, FTextureInfo& Info, const GouraudVertex* Pts, int NumPts, uint32_t PolyFlags)
{
	std::cout << "OpenGLRenderDevice::DrawGouraudPolygon(FSceneNode* Frame, FTextureInfo& Info, const GouraudVertex* Pts, int NumPts, uint32_t PolyFlags)" << std::endl;
}

void OpenGLRenderDevice::DrawTile(FSceneNode* Frame, FTextureInfo& Info,
			  float X, float Y, float XL, float YL, 
			  float U, float V, float UL, float VL, float Z, 
			  vec4 Color, vec4 Fog, uint32_t PolyFlags)
{

        GLuint shader_program = Shaders->sceneShader->ProgramID;
        GLint vertexSlot = glGetAttribLocation(shader_program, "vertex");

	    GLsizei indicesCount = 0;
	    GLsizeiptr verticesBufferSize, indicesBufferSize;
	    GLuint textureBinding = 0;

		GLfloat scale = 0.075;

		tileCount += 1;

		GLfloat vertices[] = {
			(GLfloat)-0.8 + tileCount*scale,0, 0, 
			(GLfloat)-0.8 + tileCount*scale + (GLfloat)0.5 * scale,1*scale, 0,
			(GLfloat)-0.8 + tileCount*scale, 1 * scale, 0,
			
			(GLfloat)-0.8 + tileCount*scale + (GLfloat)0.5 * scale, 0, 0, 
			(GLfloat)-0.8 + tileCount*scale, 0, 0,
			(GLfloat)-0.8 + tileCount*scale + (GLfloat)0.5 * scale,1*scale, 0
		};
		GLint  vertexCoordsCount = 6;

		GLuint indices[] = {
			0,1,2
		};
		indicesCount = 3;


    GLuint vbo;
    GLint pos;

	pos = glGetAttribLocation(shader_program, "vertex");

    //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glViewport(0, 0, 1920, 1080);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof (vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*) 0);
    glEnableVertexAttribArray(pos);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shader_program);
    glDrawArrays(GL_TRIANGLES, 0, vertexCoordsCount);

    glDeleteBuffers(1, &vbo);
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

// void OpenGLRenderDevice::DrawScene()
// {
// 	std::cout << "OpenGLRenderDevice::DrawScene()" << std::endl;

// 	while (!CommandBuffer.empty())
// 	{
// 		Draw(CommandBuffer.front());
// 		CommandBuffer.pop_front();
// 	}

// 	glFinish();

// 	if (SDL2Window::currentWindow == nullptr) {
// 		std::cout << "SDL2Window::currentWindow: NULL!!!!" << std::endl;
// 		exit(1);
// 	}
// 	SDL_GL_SwapWindow(SDL2Window::currentWindow);	
// }

// void OpenGLRenderDevice::Draw(GLDrawCommand& drawCommand)
// {
// 	std::cout << "OpenGLRenderDevice::Draw" << std::endl;
// 	drawCommand.va->Bind();
// 	drawCommand.ib->Bind();
// 	drawCommand.shader->Bind();
// 	glDrawElements(GL_TRIANGLE_STRIP, drawCommand.ib->IndicesCount(), GL_UNSIGNED_INT, drawCommand.ib->IndicesData());
// }
