
#include "Precomp.h"
#include "RenderDevice.h"

#if __EMSCRIPTEN__
#include "OpenGL/OpenGLRenderDevice.h"
#else
#include "Vulkan/VulkanRenderDevice.h"
#endif


#include <iostream>

std::unique_ptr<RenderDevice> RenderDevice::Create(GameWindow* viewport)
{
#if __EMSCRIPTEN__
	return std::make_unique<OpenGLRenderDevice>(viewport);
#else
	return std::make_unique<VulkanRenderDevice>(viewport, VulkanRenderDevice::surface);
#endif
}
