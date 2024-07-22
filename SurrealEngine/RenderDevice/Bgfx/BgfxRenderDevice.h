#pragma once

#include "RenderDevice/RenderDevice.h"

class BgfxRenderDevice: public RenderDevice {

public:
    BgfxRenderDevice(GameWindow* InWindow);
	~BgfxRenderDevice();
	void Flush(bool AllowPrecache);
	bool Exec(std::string Cmd, OutputDevice& Ar);
	void Lock(vec4 FlashScale, vec4 FlashFog, vec4 ScreenClear);
	void Unlock(bool Blit);
	void DrawComplexSurface(FSceneNode* Frame, FSurfaceInfo& Surface, FSurfaceFacet& Facet);
	void DrawGouraudPolygon(FSceneNode* Frame, FTextureInfo& Info, const GouraudVertex* Pts, int NumPts, uint32_t PolyFlags);
	void DrawTile(FSceneNode* Frame, FTextureInfo& Info, float X, float Y, float XL, float YL, float U, float V, float UL, float VL, float Z, vec4 Color, vec4 Fog, uint32_t PolyFlags);
	void Draw3DLine(FSceneNode* Frame, vec4 Color, vec3 P1, vec3 P2);
	void Draw2DLine(FSceneNode* Frame, vec4 Color, vec3 P1, vec3 P2);
	void Draw2DPoint(FSceneNode* Frame, vec4 Color, float X1, float Y1, float X2, float Y2, float Z);
	void ClearZ(FSceneNode* Frame);
	void ReadPixels(FColor* Pixels);
	void EndFlash();
	void SetSceneNode(FSceneNode* Frame);
	void PrecacheTexture(FTextureInfo& Info, uint32_t PolyFlags);
	bool SupportsTextureFormat(TextureFormat Format);
	void UpdateTextureRect(FTextureInfo& Info, int U, int V, int UL, int VL);
};