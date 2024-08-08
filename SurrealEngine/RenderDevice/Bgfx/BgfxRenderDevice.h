#pragma once

#include "RenderDevice/RenderDevice.h"
#include <bgfx/bgfx.h>

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

private:
        struct Vertex3D_UV
        {
            float x;
            float y;
            float z;
            float u;
            float v;

            static void init()
            {
                Vertex3D_UV::ms_layout
                    .begin()
                    .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
                    .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
                    .end();
            };

            static bgfx::VertexLayout ms_layout;
        };

	std::vector<std::vector<Vertex3D_UV>> vertices;

	bgfx::ProgramHandle drawTileProgram;

	bgfx::TextureHandle texture;
	bgfx::UniformHandle s_texture0;

	std::vector<bgfx::VertexBufferHandle> vertexBuffers;
	std::vector<bgfx::IndexBufferHandle> indexBuffers;
	std::vector<bgfx::UniformHandle> textureUniforms;
	std::vector<bgfx::TextureHandle> textureHandles;

	std::vector<char> drawTileVertexShaderCode;
	std::vector<char> drawTileFragmentShaderCode;

	std::vector<char> draw3DVertexShaderCode;
	std::vector<char> draw3DFragmentShaderCode;

	std::chrono::milliseconds renderingStartDate;


	std::vector<Vertex3D_UV> vertices2D;
	bgfx::VertexBufferHandle vertexBufferHandle2D;

};