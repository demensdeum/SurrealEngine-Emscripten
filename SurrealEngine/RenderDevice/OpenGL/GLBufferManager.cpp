#include "GLBufferManager.h"

GLBufferManager::GLBufferManager()
{
	VertexArray = std::make_unique<GLVertexArray>();
	IndexBuffer = std::make_unique<GLIndexBuffer>();
}