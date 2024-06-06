#pragma once

#include "GLObjects.h"
#include "GLShader.h"

#include <memory>

class GLDrawCommand
{
public:
	GLDrawCommand(GLVertexArray* va, GLIndexBuffer* ib, GLShader* shader): va(va), ib(ib), shader(shader) {}

	GLVertexArray* va;
	GLIndexBuffer* ib;
	GLShader* shader;
};