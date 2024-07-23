#include "BgfxRenderDevice.h"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <stdexcept>
#include "Window/SDL2/SDL2Window.h"
#include <SDL2/SDL_syswm.h>
#include <iostream>

BgfxRenderDevice::BgfxRenderDevice(GameWindow *InWindow)
{
        auto window = SDL2Window::currentWindow;

        if (!window)
        {
                throw std::runtime_error(std::string("Can't get SDL2Window::currentWindow"));
        }

        SDL_SysWMinfo wmi;
        SDL_VERSION(&wmi.version);
        if (!SDL_GetWindowWMInfo(window, &wmi))
        {
                throw std::runtime_error(std::string("Can't get wmi"));
        }

        bgfx::PlatformData platformData{};
#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
        platformData.ndt = wmi.info.x11.display;
        platformData.nwh = (void *)(uintptr_t)wmi.info.x11.window;
#elif BX_PLATFORM_OSX
        pd.ndt = NULL;
        pd.nwh = wmi.info.cocoa.window;
#elif BX_PLATFORM_WINDOWS
        pd.ndt = NULL;
        pd.nwh = wmi.info.win.window;
#elif BX_PLATFORM_STEAMLINK
        pd.ndt = wmi.info.vivante.display;
        pd.nwh = wmi.info.vivante.window;
#endif
        platformData.context = NULL;
        platformData.backBuffer = NULL;
        platformData.backBufferDS = NULL;

        bgfx::Init init;
        init.type = bgfx::RendererType::Vulkan;
        init.resolution.width = 1920;
        init.resolution.height = 1080;
        init.resolution.reset = BGFX_RESET_VSYNC;
        init.platformData = platformData;

        if (!bgfx::init(init))
        {
                throw std::runtime_error("Failed to initialize bgfx");
        }

        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x443355FF, 1.0f, 0);
        bgfx::setViewRect(0, 0, 0, 1920, 1080);        
}

BgfxRenderDevice::~BgfxRenderDevice()
{
}

void BgfxRenderDevice::Flush(bool AllowPrecache)
{
}

bool BgfxRenderDevice::Exec(std::string Cmd, OutputDevice &Ar) { return false; }

void BgfxRenderDevice::Lock(vec4 FlashScale, vec4 FlashFog, vec4 ScreenClear)
{
        auto now = std::chrono::system_clock::now();
        auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
        renderingStartDate = now_ms.time_since_epoch();
}

void BgfxRenderDevice::Unlock(bool Blit)
{
        if (Blit) {
                bgfx::touch(0);
                bgfx::frame();

                auto now = std::chrono::system_clock::now();
                auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
                        std::chrono::milliseconds renderingEndDate = now_ms.time_since_epoch();

                        std::chrono::milliseconds duration = renderingEndDate - renderingStartDate;
                double fps = 1000.0 / duration.count();

                std::cout << "Rendering duration: " << duration.count() << " ms" << std::endl;
                std::cout << "FPS: " << fps << std::endl;                
        }
}

void BgfxRenderDevice::DrawComplexSurface(FSceneNode *Frame, FSurfaceInfo &Surface, FSurfaceFacet &Facet)
{

}

void BgfxRenderDevice::DrawGouraudPolygon(FSceneNode *Frame, FTextureInfo &Info, const GouraudVertex *Pts, int NumPts, uint32_t PolyFlags)
{
}

void BgfxRenderDevice::DrawTile(FSceneNode *Frame, FTextureInfo &Info, float X, float Y, float XL, float YL, float U, float V, float UL, float VL, float Z, vec4 Color, vec4 Fog, uint32_t PolyFlags)
{
}

void BgfxRenderDevice::Draw3DLine(FSceneNode *Frame, vec4 Color, vec3 P1, vec3 P2)
{
}

void BgfxRenderDevice::Draw2DLine(FSceneNode *Frame, vec4 Color, vec3 P1, vec3 P2)
{
}

void BgfxRenderDevice::Draw2DPoint(FSceneNode *Frame, vec4 Color, float X1, float Y1, float X2, float Y2, float Z)
{
}

void BgfxRenderDevice::ClearZ(FSceneNode *Frame)
{
}

void BgfxRenderDevice::ReadPixels(FColor *Pixels)
{
}

void BgfxRenderDevice::EndFlash()
{
}

void BgfxRenderDevice::SetSceneNode(FSceneNode *Frame)
{
}

void BgfxRenderDevice::PrecacheTexture(FTextureInfo &Info, uint32_t PolyFlags)
{
}

bool BgfxRenderDevice::SupportsTextureFormat(TextureFormat Format)
{
        return false;
}

void BgfxRenderDevice::UpdateTextureRect(FTextureInfo &Info, int U, int V, int UL, int VL)
{
}