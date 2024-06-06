#include "GLFramebufferManager.h"

GLFrameBufferManager::GLFrameBufferManager(OpenGLRenderDevice* renderDevice) : renderDevice(renderDevice)
{
}

void GLFrameBufferManager::CreateSceneFrameBuffer()
{
	SceneFrameBuffer = std::make_unique<GLFrameBuffer>(renderDevice);
}

void GLFrameBufferManager::DestroySceneFrameBuffer()
{
	SceneFrameBuffer.reset();
}
