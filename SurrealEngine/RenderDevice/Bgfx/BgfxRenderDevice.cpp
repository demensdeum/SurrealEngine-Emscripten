#include "BgfxRenderDevice.h"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <stdexcept>
#include "Window/SDL2/SDL2Window.h"
#include <SDL2/SDL_syswm.h>
#include <fstream>
#include <iostream>

#define BGFX_MODE_OPENGL 1
#define BGFX_MODE_VULKAN 2

#define BGFX_MODE BGFX_MODE_OPENGL

bgfx::VertexLayout BgfxRenderDevice::Vertex3D_UV::ms_layout;

const int framebufferWidth = 1920;
const int framebufferHeight = 1080;

std::map<uint64_t, bgfx::TextureHandle> texturesCache;

std::vector<char> readFile(const std::string &filename)
{
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
                std::string error = "failed to open file: " + filename;
                throw std::runtime_error(error);
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
}

bgfx::TextureHandle loadTexture(const char *filepath, uint64_t cacheID)
{
        SDL_Surface *surface = SDL_LoadBMP(filepath);
        if (!surface)
        {
                throw std::runtime_error("Failed to load texture: " + std::string(SDL_GetError()));
        }

        const bgfx::Memory *mem = bgfx::alloc(surface->w * surface->h * 4);
        uint8_t *dst = (uint8_t *)mem->data;
        uint8_t *src = (uint8_t *)surface->pixels;

        for (int y = 0; y < surface->h; ++y)
        {
                for (int x = 0; x < surface->w; ++x)
                {
                        uint8_t *pixel = &src[(y * surface->pitch) + (x * surface->format->BytesPerPixel)];
                        dst[(y * surface->w + x) * 4 + 0] = 256 + cacheID; // R
                        dst[(y * surface->w + x) * 4 + 1] = pixel[1]; // G
                        dst[(y * surface->w + x) * 4 + 2] = pixel[0]; // B
                        dst[(y * surface->w + x) * 4 + 3] = 255;      // A
                }
        }

        bgfx::TextureHandle textureHandle = bgfx::createTexture2D(
            uint16_t(surface->w),
            uint16_t(surface->h),
            false,
            1,
            bgfx::TextureFormat::RGBA8,
            0,
            mem);

        SDL_FreeSurface(surface);
        return textureHandle;
}

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
#if BGFX_MODE == BGFX_MODE_OPENGL
        init.type = bgfx::RendererType::OpenGL;
#elif BGFX_MODE == BGFX_MODE_VULKAN
        init.type = bgfx::RendererType::Vulkan;
#endif
        init.resolution.width = framebufferWidth;
        init.resolution.height = framebufferHeight;
        init.resolution.reset = BGFX_RESET_VSYNC;
        init.platformData = platformData;

        if (!bgfx::init(init))
        {
                throw std::runtime_error("Failed to initialize bgfx");
        }

        Vertex3D_UV::init();

        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x443355FF, 1.0f, 0);
        bgfx::setViewRect(0, 0, 0, framebufferWidth, framebufferHeight);

#if BGFX_MODE == BGFX_MODE_OPENGL
        drawTileVertexShaderCode = readFile("BgfxRenderDeviceDrawTileVertex.glsl");
#elif BGFX_MODE == BGFX_MODE_VULKAN
        drawTileVertexShaderCode = readFile("BgfxRenderDeviceDrawTileVertex.spirv");
#endif

        bgfx::ShaderHandle vertexShader = bgfx::createShader(bgfx::makeRef(drawTileVertexShaderCode.data(), drawTileVertexShaderCode.size()));
        if (!bgfx::isValid(vertexShader))
        {
                throw std::runtime_error("Failed to create vertex shader");
        }
        else
        {
                std::cout << "Vertex shader load success!" << std::endl;
        }

#if BGFX_MODE == BGFX_MODE_OPENGL
        drawTileFragmentShaderCode = readFile("BgfxRenderDeviceDrawTileFragment.glsl");
#elif BGFX_MODE == BGFX_MODE_VULKAN
        drawTileFragmentShaderCode = readFile("BgfxRenderDeviceDrawTileFragment.spirv");
#endif

        bgfx::ShaderHandle fragmentShader = bgfx::createShader(bgfx::makeRef(drawTileFragmentShaderCode.data(), drawTileFragmentShaderCode.size()));
        if (!bgfx::isValid(fragmentShader))
        {
                throw std::runtime_error("Failed to create fragment shader");
        }
        else
        {
                std::cout << "Fragment shader load success!" << std::endl;
        }

        drawTileProgram = bgfx::createProgram(vertexShader, fragmentShader, true);

        if (!bgfx::isValid(drawTileProgram))
        {
                throw std::runtime_error("Failed to create program");
        }
        s_texture0 = bgfx::createUniform("s_texture0", bgfx::UniformType::Sampler);
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
        if (Blit)
        {
                // draw tiles
                std::vector<bgfx::VertexBufferHandle> tileVertexBufferHandles;
                std::vector<std::vector<Vertex3D_UV> *> tilesVectorsToDelete;

                for (size_t i = 0; i < vertices2D.size(); i += 6)
                {
                        std::vector<Vertex3D_UV> *tileVector = new std::vector<Vertex3D_UV>();
                        tilesVectorsToDelete.push_back(tileVector);
                        for (size_t j = i; j < i + 6 && j < vertices2D.size(); ++j) {
                                Vertex3D_UV vertex = vertices2D[j];
                                tileVector->push_back(vertex);
                        }
                        auto sliceBufferHandle2D = bgfx::createVertexBuffer(
                                bgfx::makeRef(
                                tileVector->data(),
                                sizeof(Vertex3D_UV) * tileVector->size()),
                                Vertex3D_UV::ms_layout
                        );   
                        tileVertexBufferHandles.push_back(sliceBufferHandle2D);
                }

                int i = 0;
                for (auto sliceHandle : tileVertexBufferHandles) {
                        bgfx::TextureHandle texture = vertices2DTileTexture[i];
                        bgfx::setVertexBuffer(0, sliceHandle);
                        bgfx::setTexture(0, s_texture0, texture);

                        bgfx::setState(BGFX_STATE_DEFAULT);

                        bgfx::submit(0, drawTileProgram);                      
                        i++;  
                }

                bgfx::frame();

                for (auto vertexSliceVector : tilesVectorsToDelete) {
                        //delete vertexSliceVector; <------- FIX MEMORY LEAK
                }
                for (auto sliceHandle : tileVertexBufferHandles) {
                        bgfx::destroy(sliceHandle);
                }

                tilesVectorsToDelete.clear();
                vertices2D.clear(); 
                vertices2DTileTexture.clear();

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

// std::vector<FColor> P8_Convert(FTextureInfo *info, size_t mipmapLevel)
// {
//         std::vector<FColor> result;

//         UnrealMipmap *mipmap = &info->Texture->Mipmaps[mipmapLevel];
//         size_t mipmapWidth = mipmap->Width;
//         size_t mipmapHeight = mipmap->Height;

//         FColor *palette = info->Palette;

//         result.resize(mipmapWidth * mipmapHeight);

//         if (info->Texture->bMasked())
//         {
//                 FColor transparent(0, 0, 0, 0);

//                 for (size_t y = 0; y < mipmapHeight; y++)
//                 {
//                         for (size_t x = 0; x < mipmapWidth; x++)
//                         {
//                                 uint8_t index = mipmap->Data[x + y * mipmapWidth];
//                                 result[x + y * mipmapWidth] = index == 0 ? transparent : palette[index];
//                         }
//                 }
//         }
//         else
//         {
//                 for (size_t y = 0; y < mipmapHeight; y++)
//                 {
//                         for (size_t x = 0; x < mipmapWidth; x++)
//                         {
//                                 uint8_t index = mipmap->Data[x + y * mipmapWidth];
//                                 result[x + y * mipmapWidth] = palette[index];
//                         }
//                 }
//         }

//         return result;
// }

// bgfx::TextureHandle convertSurfaceToTexture(SDL_Surface *surface)
// {
//         if (!surface)
//         {
//                 throw std::runtime_error("convertSurfaceToTexture error: surface is null");
//         }

//         const bgfx::Memory *mem = bgfx::alloc(surface->w * surface->h * 4);
//         uint8_t *dst = (uint8_t *)mem->data;
//         uint8_t *src = (uint8_t *)surface->pixels;

//         for (int y = 0; y < surface->h; ++y)
//         {
//                 for (int x = 0; x < surface->w; ++x)
//                 {
//                         uint8_t *pixel = &src[(y * surface->pitch) + (x * surface->format->BytesPerPixel)];
//                         dst[(y * surface->w + x) * 4 + 0] = pixel[2]; // R
//                         dst[(y * surface->w + x) * 4 + 1] = pixel[1]; // G
//                         dst[(y * surface->w + x) * 4 + 2] = pixel[0]; // B
//                         dst[(y * surface->w + x) * 4 + 3] = 255;      // A
//                 }
//         }

//         bgfx::TextureHandle textureHandle = bgfx::createTexture2D(
//             uint16_t(surface->w),
//             uint16_t(surface->h),
//             false,
//             1,
//             bgfx::TextureFormat::RGBA8,
//             0,
//             mem);
//         return textureHandle;
// }

void BgfxRenderDevice::bindTexture(FTextureInfo *texture) {
	bgfx::TextureHandle textureBinding;

	auto textureWidth = texture->Mips[0].Width;
	auto textureHeight = texture->Mips[0].Height;

	if (texturesCache.find(texture->CacheID) == texturesCache.end()) {
		texturesCache[texture->CacheID] = loadTexture("brick.texture.bmp", texture->CacheID);
                textureBinding = texturesCache[texture->CacheID];
	}
	else {
		textureBinding = texturesCache[texture->CacheID];
	}

        vertices2DTileTexture.push_back(textureBinding);
}

void BgfxRenderDevice::DrawTile(
    FSceneNode *Frame,
    FTextureInfo &Info,
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
    vec4 Fog, uint32_t PolyFlags)
{
    auto textureWidth = Info.Mips[0].Width;
    auto textureHeight = Info.Mips[0].Height;

    float u = U / textureWidth;
    float v = V / textureHeight;
    float ul = UL / textureWidth;
    float vl = VL / textureHeight;    

        float ZZZ = 0.0f;

        float left = XL / framebufferWidth - 0.5f;
        float right = X / framebufferWidth - 0.5f;
        
        float top = YL / framebufferHeight - 0.5f;
        float bottom = Y / framebufferHeight - 0.5f;

        vertices2D.push_back(Vertex3D_UV{left, bottom, ZZZ, 0.0f, 0.0f});
        vertices2D.push_back(Vertex3D_UV{right, bottom, ZZZ, 1.0f, 0.0f});
        vertices2D.push_back(Vertex3D_UV{left, top, ZZZ, 0.0f, 1.0f});

        vertices2D.push_back(Vertex3D_UV{right, bottom, ZZZ, 1.0f, 0.0f});
        vertices2D.push_back(Vertex3D_UV{right, top, ZZZ, 1.0f, 1.0f});
        vertices2D.push_back(Vertex3D_UV{left, top, ZZZ, 0.0f, 1.0f});

        bindTexture(&Info);
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