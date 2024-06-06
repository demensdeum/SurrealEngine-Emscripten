#pragma once

#include "Math/mat.h"
#include "Math/vec.h"

// Stuff copied from Vulkan renderer. This can be generalized perhaps (move the "common" things to a new "RenderDevice/Common" location)

struct GLSceneVertex
{
	uint32_t Flags;
	vec3 Position;
	vec2 TexCoord;
	vec2 TexCoord2;
	vec2 TexCoord3;
	vec2 TexCoord4;
	vec4 Color;
	ivec4 TextureBinds;
};

struct GLScenePushConstants
{
	mat4 objectToProjection;
};