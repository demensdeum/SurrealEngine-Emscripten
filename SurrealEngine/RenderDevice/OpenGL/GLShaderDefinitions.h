#pragma once

// Shaders taken from VulkanRenderDevice, and modified a bit to fit OpenGL 3.3

const std::string vertexShaderCode = R"(
	#version 420

	uniform mat4 objectToProjection;

	layout(location = 0) in uint aFlags;
	layout(location = 1) in vec3 aPosition;
	layout(location = 2) in vec2 aTexCoord;
	layout(location = 3) in vec2 aTexCoord2;
	layout(location = 4) in vec2 aTexCoord3;
	layout(location = 5) in vec2 aTexCoord4;
	layout(location = 6) in vec4 aColor;

	layout(location = 0) flat out uint flags;
	layout(location = 1) out vec2 texCoord;
	layout(location = 2) out vec2 texCoord2;
	layout(location = 3) out vec2 texCoord3;
	layout(location = 4) out vec2 texCoord4;
	layout(location = 5) out vec4 color;

	void main()
	{
		gl_Position = objectToProjection * vec4(aPosition, 1.0);
		flags = aFlags;
		texCoord = aTexCoord;
		texCoord2 = aTexCoord2;
		texCoord3 = aTexCoord3;
		texCoord4 = aTexCoord4;
		color = aColor;
	}
)";

const std::string fragmentShaderCode = R"(
	#version 420

	uniform sampler2D tex;
	uniform sampler2D texLightmap;
	uniform sampler2D texMacro;
	uniform sampler2D texDetail;

	layout(location = 0) flat in uint flags;
	layout(location = 1) centroid in vec2 texCoord;
	layout(location = 2) in vec2 texCoord2;
	layout(location = 3) in vec2 texCoord3;
	layout(location = 4) in vec2 texCoord4;
	layout(location = 5) in vec4 color;

	layout(location = 0) out vec4 outColor;

	vec3 linear(vec3 c)
	{
		return mix(c / 12.92, pow((c + 0.055) / 1.055, vec3(2.4)), step(c, vec3(0.04045)));
	}

	vec4 darkClamp(vec4 c)
	{
		// Make all textures a little darker as some of the textures (i.e coronas) never become completely black as they should have
		float cutoff = 3.1/255.0;
		return vec4(clamp((c.rgb - cutoff) / (1.0 - cutoff), 0.0, 1.0), c.a);
	}

	vec4 textureTex(vec2 uv) { return texture(tex, uv); }
	vec4 textureMacro(vec2 uv) { return texture(texMacro, uv); }
	vec4 textureLightmap(vec2 uv) { return texture(texLightmap, uv); }
	vec4 textureDetail(vec2 uv) { return texture(texDetail, uv); }

	void main()
	{
		float actorXBlending = (flags & 32) != 0 ? 1.5 : 1.0;
		float oneXBlending = (flags & 64) != 0 ? 1.0 : 2.0;

		outColor = darkClamp(textureTex(texCoord)) * darkClamp(color) * actorXBlending;

		if ((flags & 2) != 0) // Macro texture
		{
			outColor *= darkClamp(textureMacro(texCoord3));
		}

		if ((flags & 1) != 0) // Lightmap
		{
			outColor.rgb *= clamp(textureLightmap(texCoord2).rgb, 0.0, 1.0) * oneXBlending;
		}

		if ((flags & 4) != 0) // Detail texture
		{
			float fadedistance = 380.0f;
			float a = clamp(2.0f - (1.0f / gl_FragCoord.w) / fadedistance, 0.0f, 1.0f);
			vec4 detailColor = (textureDetail(texCoord4) - 0.5) * 0.8 + 1.0;
			outColor.rgb = mix(outColor.rgb, outColor.rgb * detailColor.rgb, a);
		}
		else if ((flags & 8) != 0) // Fog map
		{
			vec4 fogcolor = textureDetail(texCoord4);
			outColor.rgb = fogcolor.rgb + outColor.rgb * (1.0 - fogcolor.a);
		}
		else if ((flags & 16) != 0) // Fog color
		{
			vec4 fogcolor = vec4(texCoord2, texCoord3);
			outColor.rgb = fogcolor.rgb + outColor.rgb * (1.0 - fogcolor.a);
		}

		#if defined(ALPHATEST)
		if (outColor.a < 0.5) discard;
		#endif

		outColor = clamp(outColor, 0.0, 1.0);
	}
)";
