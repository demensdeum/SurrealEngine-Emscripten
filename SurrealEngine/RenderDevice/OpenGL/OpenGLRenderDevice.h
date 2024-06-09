#pragma once

#include <memory>
#include <deque>

#include "RenderDevice/RenderDevice.h"

#include "GLTextureManager.h"
#include "GLShaderManager.h"
#include "GLFramebufferManager.h"
#include "GLDrawCommand.h"

#include <GL/glew.h>

class OpenGLRenderDevice : public RenderDevice
{
public:
	OpenGLRenderDevice(GameWindow* InWindow);
	~OpenGLRenderDevice();

	void Flush(bool AllowPrecache) override; // free all resources

	void DrawTile(
		FSceneNode* Frame, 
		FTextureInfo& Info, 
		float X, 
		float Y, 
		float XL, 
		float YL, 
		float U, 
		float V, 
		float UL, 
		float VL, 
		float Z, 
		vec4 Color, 
		vec4 Fog, 
		uint32_t PolyFlags
	) override; // draw 2d cards on screen

	void Lock(
		vec4 FlashScale,
		vec4 FlashFog, 
		vec4 ScreenClear
	) override; // Pre render screen clear
	
	void Unlock(bool Blit) override;



	bool Exec(std::string Cmd, OutputDevice& Ar) override;
	void DrawComplexSurface(FSceneNode* Frame, FSurfaceInfo& Surface, FSurfaceFacet& Facet) override;
	void DrawGouraudPolygon(FSceneNode* Frame, FTextureInfo& Info, const GouraudVertex* Pts, int NumPts, uint32_t PolyFlags) override;
	void Draw3DLine(FSceneNode* Frame, vec4 Color, vec3 P1, vec3 P2) override;
	void Draw2DLine(FSceneNode* Frame, vec4 Color, vec3 P1, vec3 P2) override;
	void Draw2DPoint(FSceneNode* Frame, vec4 Color, float X1, float Y1, float X2, float Y2, float Z) override;
	void ClearZ(FSceneNode* Frame) override;
	void ReadPixels(FColor* Pixels) override;
	void EndFlash() override;
	void SetSceneNode(FSceneNode* Frame) override;
	void PrecacheTexture(FTextureInfo& Info, uint32_t PolyFlags) override;
	bool SupportsTextureFormat(TextureFormat Format) override;
	void UpdateTextureRect(FTextureInfo& Info, int U, int V, int UL, int VL) override;
private:
	FSceneNode* CurrentFrame = nullptr;

	float Aspect;

	std::unique_ptr<GLFrameBufferManager> Framebuffers;
	std::unique_ptr<GLTextureManager> Textures;
	std::unique_ptr<GLShaderManager> Shaders;
	std::deque<GLDrawCommand> CommandBuffer;
};