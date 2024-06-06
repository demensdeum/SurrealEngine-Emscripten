#pragma once

#include <vector>
#include <memory>

#include "SceneData.h"
#include <GL/glew.h>

class OpenGLRenderDevice;

class GLVertexBuffer
{
public:
	GLVertexBuffer();
	~GLVertexBuffer();

	void Bind();
	void Unbind();

	void LoadBufferData(const std::vector<GLSceneVertex>& vertices);

private:
	GLuint vertexBufferID;
};

class GLVertexArray
{
public:
	GLVertexArray();
	~GLVertexArray();

	void Bind();
	void Unbind();

	void LoadVertexData(const std::vector<GLSceneVertex>& vertices);

private:
	void CreateVertexArrayAttribs();
	GLuint vertexArrayID;
	std::unique_ptr<GLVertexBuffer> vertexBufferObject;
};

class GLIndexBuffer
{
public:
	GLIndexBuffer();
	~GLIndexBuffer();

	void Bind();
	void Unbind();

	void LoadIndicesData(const std::vector<unsigned int>& indices);
	size_t IndicesCount() { return indices.size(); }
	GLuint* IndicesData() { return indices.data(); }

private:
	GLuint IndexArrayID;
	std::vector<unsigned int> indices;
};

class GLFrameBuffer
{
public:
	GLFrameBuffer(OpenGLRenderDevice* renderDevice);
	~GLFrameBuffer();

	void Bind();
	void Unbind();

	void UpdateFrameBufferData();

	std::vector<uint8_t> ReadPixelData();

private:
	void CreateFrameBufferData();

	GLuint FramebufferID;
	GLuint FramebufferTextureID;
	GLuint RenderbufferID;
	OpenGLRenderDevice* renderDevice;
};
