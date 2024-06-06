
#include "Precomp.h"
#include "RenderDevice.h"
#include "Vulkan/VulkanRenderDevice.h"
#include "OpenGL/OpenGLRenderDevice.h"

#include <iostream>

std::unique_ptr<RenderDevice> RenderDevice::Create(GameWindow* viewport)
{
	return std::make_unique<OpenGLRenderDevice>(viewport);
}

std::unique_ptr<RenderDevice> RenderDevice::CreateUnused(GameWindow* viewport, std::shared_ptr<VulkanSurface> surface) {
	return std::make_unique<VulkanRenderDevice>(viewport, surface);
}
