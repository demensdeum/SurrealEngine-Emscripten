
#include "Precomp.h"
#include "RenderDevice.h"
#include "Vulkan/VulkanRenderDevice.h"
#include "OpenGL/OpenGLRenderDevice.h"

std::unique_ptr<RenderDevice> RenderDevice::Create(GameWindow* viewport, std::shared_ptr<VulkanSurface> surface)
{
#ifdef EMSCRIPTEN
	return std::make_unique<OpenGLRenderDevice>(viewport);
#else
	return std::make_unique<VulkanRenderDevice>(viewport, surface);
#endif
}
