#include "OpenGLRenderDevice.h"
#include "Window/Window.h"

#include "GLTexture.h"

#include <stdexcept>

#include "Window/SDL2/SDL2Window.h"
#include <SDL2/SDL.h>
#include <iostream>

// TODO: Implement the OpenGL renderer :V

OpenGLRenderDevice::OpenGLRenderDevice(GameWindow* InWindow)
{
	Viewport = InWindow;

	Framebuffers.reset(new GLFrameBufferManager(this));
	Textures.reset(new GLTextureManager());
	Shaders.reset(new GLShaderManager());
}

OpenGLRenderDevice::~OpenGLRenderDevice()
{
	Textures->ClearTextures();
	Framebuffers.reset();
	Shaders.reset();
}

void OpenGLRenderDevice::Flush(bool AllowPrecache)
{

}

bool OpenGLRenderDevice::Exec(std::string Cmd, OutputDevice& Ar)
{
	return false;
}

void OpenGLRenderDevice::Lock(vec4 FlashScale, vec4 FlashFog, vec4 ScreenClear)
{

}

void OpenGLRenderDevice::Unlock(bool Blit)
{

}

void OpenGLRenderDevice::DrawComplexSurface(FSceneNode* Frame, FSurfaceInfo& Surface, FSurfaceFacet& Facet)
{

}

void OpenGLRenderDevice::DrawGouraudPolygon(FSceneNode* Frame, FTextureInfo& Info, const GouraudVertex* Pts, int NumPts, uint32_t PolyFlags)
{

}

void OpenGLRenderDevice::DrawTile(FSceneNode* Frame, FTextureInfo& Info,
			  float X, float Y, float XL, float YL, 
			  float U, float V, float UL, float VL, float Z, 
			  vec4 Color, vec4 Fog, uint32_t PolyFlags)
{

}

void OpenGLRenderDevice::Draw3DLine(FSceneNode* Frame, vec4 Color, vec3 P1, vec3 P2)
{

}

void OpenGLRenderDevice::Draw2DLine(FSceneNode* Frame, vec4 Color, vec3 P1, vec3 P2)
{

}

void OpenGLRenderDevice::Draw2DPoint(FSceneNode* Frame, vec4 Color, float X1, float Y1, float X2, float Y2, float Z)
{

}

void OpenGLRenderDevice::ClearZ(FSceneNode* Frame)
{
	// Following what the Vulkan driver did, seems like it disregards the Frame parameter
	// and just draws the scene then clears the depth buffer
	DrawScene();

	glClearDepth(1.0f);
	glClear(GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderDevice::ReadPixels(FColor* Pixels)
{
	auto readPixels = Framebuffers->SceneFrameBuffer->ReadPixelData();

	memcpy(Pixels, readPixels.data(), readPixels.size());
}

void OpenGLRenderDevice::EndFlash()
{

}

void OpenGLRenderDevice::SetSceneNode(FSceneNode* Frame)
{
	DrawScene();

	CurrentFrame = Frame;

	Aspect = Frame->FY / Frame->FX;

	glViewport(Frame->X, Frame->Y, Frame->XB, Frame->YB);
}

void OpenGLRenderDevice::PrecacheTexture(FTextureInfo& Info, uint32_t PolyFlags)
{
	// TODO: Handle PolyFlags too
	Textures->GetTexture(&Info);
}

bool OpenGLRenderDevice::SupportsTextureFormat(TextureFormat Format)
{
	return GLTexture::TextureFormatToGL(Format);
}

void OpenGLRenderDevice::UpdateTextureRect(FTextureInfo& Info, int U, int V, int UL, int VL)
{

}

void OpenGLRenderDevice::DrawScene()
{
	// Following what the Vulkan driver does:
	// Draw*() functions actually add vertices/colors/indexes to the scene
	// Then DrawScene() (DrawBatch() in VulkanRenderDevice) handles the actual drawing
	// TODO: Actually do that :V

	while (!CommandBuffer.empty())
	{
		Draw(CommandBuffer.front());
		CommandBuffer.pop_front();
	}

	glFinish();

	if (SDL2Window::currentWindow == nullptr) {
		std::cout << "SDL2Window::currentWindow: NULL!!!!" << std::endl;
		exit(1);
	}
	SDL_GL_SwapWindow(SDL2Window::currentWindow);	
}

void OpenGLRenderDevice::Draw(GLDrawCommand& drawCommand)
{
	drawCommand.va->Bind();
	drawCommand.ib->Bind();
	drawCommand.shader->Bind();
	glDrawElements(GL_TRIANGLE_STRIP, drawCommand.ib->IndicesCount(), GL_UNSIGNED_INT, drawCommand.ib->IndicesData());
}
