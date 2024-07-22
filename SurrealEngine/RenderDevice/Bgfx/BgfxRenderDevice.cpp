#include "BgfxRenderDevice.h"

BgfxRenderDevice::BgfxRenderDevice(GameWindow *InWindow) {

}

BgfxRenderDevice::~BgfxRenderDevice() {

}

void BgfxRenderDevice::Flush(bool AllowPrecache) {

}

bool BgfxRenderDevice::Exec(std::string Cmd, OutputDevice &Ar) { return false; }

void BgfxRenderDevice::Lock(vec4 FlashScale, vec4 FlashFog, vec4 ScreenClear) {

}

void BgfxRenderDevice::Unlock(bool Blit) {

}

void BgfxRenderDevice::DrawComplexSurface(FSceneNode *Frame, FSurfaceInfo &Surface, FSurfaceFacet &Facet) {

}

void BgfxRenderDevice::DrawGouraudPolygon(FSceneNode *Frame, FTextureInfo &Info, const GouraudVertex *Pts, int NumPts, uint32_t PolyFlags) {

}

void BgfxRenderDevice::DrawTile(FSceneNode *Frame, FTextureInfo &Info, float X, float Y, float XL, float YL, float U, float V, float UL, float VL, float Z, vec4 Color, vec4 Fog, uint32_t PolyFlags) {

}

void BgfxRenderDevice::Draw3DLine(FSceneNode *Frame, vec4 Color, vec3 P1, vec3 P2) {

}

void BgfxRenderDevice::Draw2DLine(FSceneNode *Frame, vec4 Color, vec3 P1, vec3 P2) {

}

void BgfxRenderDevice::Draw2DPoint(FSceneNode *Frame, vec4 Color, float X1, float Y1, float X2, float Y2, float Z) {

}

void BgfxRenderDevice::ClearZ(FSceneNode *Frame) {

}

void BgfxRenderDevice::ReadPixels(FColor *Pixels) {

}

void BgfxRenderDevice::EndFlash() {

}

void BgfxRenderDevice::SetSceneNode(FSceneNode *Frame) {

}

void BgfxRenderDevice::PrecacheTexture(FTextureInfo &Info, uint32_t PolyFlags) {

}

bool BgfxRenderDevice::SupportsTextureFormat(TextureFormat Format) {
    return false;
}

void BgfxRenderDevice::UpdateTextureRect(FTextureInfo &Info, int U, int V, int UL, int VL) {

}