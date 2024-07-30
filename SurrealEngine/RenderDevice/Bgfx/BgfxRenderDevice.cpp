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

bgfx::TextureHandle loadTexture(const char *filepath)
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
                        dst[(y * surface->w + x) * 4 + 0] = pixel[2]; // R
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
        init.resolution.width = 1920;
        init.resolution.height = 1080;
        init.resolution.reset = BGFX_RESET_VSYNC;
        init.platformData = platformData;

        if (!bgfx::init(init))
        {
                throw std::runtime_error("Failed to initialize bgfx");
        }

        Vertex3D_UV::init();

        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x443355FF, 1.0f, 0);
        bgfx::setViewRect(0, 0, 0, 1920, 1080);

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

        texture = loadTexture("brick.texture.bmp");
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
                bgfx::frame();

                for (auto vbh : vertexBuffers)
                {
                        bgfx::destroy(vbh);
                }
                vertexBuffers.clear();

                vertices.clear();

                for (auto ibh : indexBuffers)
                {
                        bgfx::destroy(ibh);
                }
                indexBuffers.clear();

                // for (auto textureHandle : textureHandles)
                // {
                //         bgfx::destroy(textureHandle);
                // }
                // textureUniforms.clear();

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

        float u = float(U) / textureWidth;
        float v = float(V) / textureHeight;
        float ul = float(UL) / textureWidth;
        float vl = float(VL) / textureHeight;

        float ndcX = (X / 1920) * 2.0f - 1.0f;
        float ndcY = (Y / 1080) * 2.0f - 1.0f;
        float ndcXL = (XL / 1920) * 2.0f;
        float ndcYL = (YL / 1080) * 2.0f;

        // Vertex3D_UV vertices[] = {
        //     {ndcX, ndcY + ndcYL, Z, u, v + vl},              // bottom-left
        //     {ndcX + ndcXL, ndcY + ndcYL, Z, u + ul, v + vl}, // bottom-right
        //     {ndcX + ndcXL, ndcY, Z, u + ul, v},              // top-right
        //     {ndcX, ndcY, Z, u, v}                            // top-left
        // };

        // Vertex3D_UV vertices[] = {
        //     {0, 0, 0.1, u, v + vl},              // bottom-left
        //     {1, 0, 0.1, u + ul, v + vl}, // bottom-right
        //     {1, 1, 0.1, u + ul, v},              // top-right
        //     {0, 1, 0.1, u, v}                            // top-left
        // };

        // Vertex3D_UV vertices[] = {
        //     {-1.0f, 1.0f, 0.0f, 0.0f, 0.0f},  // Top-left
        //     {1.0f, 1.0f, 0.0f, 1.0f, 0.0f},   // Top-right
        //     {-1.0f, -1.0f, 0.0f, 0.0f, 1.0f}, // Bottom-left
        //     {1.0f, -1.0f, 0.0f, 1.0f, 1.0f}   // Bottom-right
        // };

        float ZZZ = -0.5f;

        vertices.push_back(std::vector<Vertex3D_UV>());

        std::vector<Vertex3D_UV> *cube = &vertices[vertices.size() - 1];

        cube->push_back({-1.0f, 1.0f, ZZZ, 0.0f, 0.0f});
        cube->push_back({1.0f, 1.0f, ZZZ, 1.0f, 0.0f});
        cube->push_back({-1.0f, -1.0f, ZZZ, 0.0f, 1.0f});

        cube->push_back({1.0f, 1.0f, ZZZ, 1.0f, 0.0f});
        cube->push_back({1.0f, -1.0f, ZZZ, 1.0f, 1.0f});
        cube->push_back({-1.0f, -1.0f, ZZZ, 0.0f, 1.0f});

        // const uint16_t indices[] =
        //     {
        //         0, 1, 2,
        //         1, 3, 2};

        // uint16_t indices[] = {0, 1, 2, 2, 3, 0};

        // Create vertex and index buffers
        vertexBuffers.push_back(bgfx::createVertexBuffer(
            bgfx::makeRef(cube->data(), sizeof(Vertex3D_UV) * cube->size()),
            Vertex3D_UV::ms_layout));
        

        // bgfx::IndexBufferHandle ibh = bgfx::createIndexBuffer(
        //     bgfx::makeRef(indices, sizeof(indices)));
        // indexBuffers.push_back(ibh);

        bgfx::setVertexBuffer(0, vertexBuffers[vertexBuffers.size()-1]);
        //bgfx::setIndexBuffer(ibh);
        bgfx::setTexture(0, s_texture0, texture);

        bgfx::submit(0, drawTileProgram);

        // SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(
        //     0,
        //     textureWidth,
        //     textureHeight,
        //     32,
        //     SDL_PIXELFORMAT_RGBA32);

        // UTexture *Texture = Info.Texture;

        // UnrealMipmap *mipmap = &Info.Mips[0];
        // uint8_t *mipmapData = mipmap->Data.data();

        // size_t miplevel = 0;

        // if (!mipmap || !mipmapData)
        // {
        //         std::cout << "mipmap or mipmapData is nullptr" << std::endl;
        //         return;
        // }

        // auto converted_data = P8_Convert(&Info, miplevel);
        // mipmapData = (uint8_t *)converted_data.data();

        // int cursor = 0;
        // for (auto i = 0; i < textureWidth * textureHeight * 4; i += 4)
        // {
        //         auto surfacePixels = (Uint8 *)surface->pixels;

        //         auto pixels = mipmapData;

        //         auto redComponent = pixels[i];
        //         auto greenComponent = pixels[i + 1];
        //         auto blueComponent = pixels[i + 2];
        //         auto alphaComponent = pixels[i + 3];

        //         surfacePixels[cursor] = redComponent;
        //         surfacePixels[cursor + 1] = greenComponent;
        //         surfacePixels[cursor + 2] = blueComponent;
        //         surfacePixels[cursor + 3] = alphaComponent;
        // }

        // bgfx::TextureHandle texture = convertSurfaceToTexture(surface);

        // SDL_FreeSurface(surface);

        // textureHandles.push_back(texture);
        // textureUniforms.push_back(s_texture0);
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