#pragma once

#include "GLObjects.h"

#include <memory>

class GLBufferManager
{
public:
	GLBufferManager();

	std::unique_ptr<GLVertexArray> VertexArray;
	std::unique_ptr<GLIndexBuffer> IndexBuffer;
};