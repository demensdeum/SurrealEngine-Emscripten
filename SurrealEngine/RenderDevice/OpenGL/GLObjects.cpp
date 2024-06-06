#include "GLObjects.h"

#include "OpenGLRenderDevice.h"

#include "Window/Window.h"

GLVertexArray::GLVertexArray()
{
	glGenVertexArrays(1, &vertexArrayID);
}

GLVertexArray::~GLVertexArray()
{
	glDeleteVertexArrays(1, &vertexArrayID);
}

void GLVertexArray::Bind()
{
	glBindVertexArray(vertexArrayID);
}

void GLVertexArray::Unbind()
{
	glBindVertexArray(NULL);
}

void GLVertexArray::LoadVertexData(const std::vector<GLSceneVertex>& vertices)
{
	Bind();
	
	vertexBufferObject = std::make_unique<GLVertexBuffer>();

	vertexBufferObject->LoadBufferData(vertices);

	CreateVertexArrayAttribs();

	Unbind();
}

void GLVertexArray::CreateVertexArrayAttribs()
{
	vertexBufferObject->Bind();

	// Layout follows the SceneVertex struct
	// uint32_t Flags
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 1, GL_UNSIGNED_INT, GL_FALSE, sizeof(uint32_t), (GLvoid*)offsetof(GLSceneVertex, Flags));

	// vec3 Position
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (GLvoid*)offsetof(GLSceneVertex, Position));

	// vec2 TexCoord
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (GLvoid*)offsetof(GLSceneVertex, TexCoord));

	// vec2 TexCoord2
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (GLvoid*)offsetof(GLSceneVertex, TexCoord2));

	// vec2 TexCoord3
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (GLvoid*)offsetof(GLSceneVertex, TexCoord3));

	// vec2 TexCoord4
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (GLvoid*)offsetof(GLSceneVertex, TexCoord4));

	// vec4 Color
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(vec4), (GLvoid*)offsetof(GLSceneVertex, Color));

	// ivec4 TextureBinds
	glEnableVertexAttribArray(7);
	glVertexAttribPointer(7, 4, GL_INT, GL_FALSE, sizeof(ivec4), (GLvoid*)offsetof(GLSceneVertex, TextureBinds));

	vertexBufferObject->Unbind();
}

//=================================================

GLVertexBuffer::GLVertexBuffer()
{
	glGenBuffers(1, &vertexBufferID);
}

GLVertexBuffer::~GLVertexBuffer()
{
	glDeleteBuffers(1, &vertexBufferID);
}

void GLVertexBuffer::Bind()
{
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
}

void GLVertexBuffer::Unbind()
{
	glBindBuffer(GL_ARRAY_BUFFER, NULL);
}

void GLVertexBuffer::LoadBufferData(const std::vector<GLSceneVertex>& vertices)
{
	Bind();

	glBufferData(GL_ARRAY_BUFFER, sizeof(GLSceneVertex) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW);

	Unbind();
}

//=================================================

GLIndexBuffer::GLIndexBuffer()
{
	glGenBuffers(1, &IndexArrayID);
}

GLIndexBuffer::~GLIndexBuffer()
{
	indices.clear();
	glDeleteBuffers(1, &IndexArrayID);
}

void GLIndexBuffer::Bind()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexArrayID);
}

void GLIndexBuffer::Unbind()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);
}

void GLIndexBuffer::LoadIndicesData(const std::vector<unsigned int>& indices)
{
	Bind();

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size(), indices.data(), GL_DYNAMIC_DRAW);

	Unbind();

	this->indices = indices;
}

//=================================================

GLFrameBuffer::GLFrameBuffer(OpenGLRenderDevice* renderDevice) : renderDevice(renderDevice)
{
	glGenFramebuffers(1, &FramebufferID);
	CreateFrameBufferData();
}

GLFrameBuffer::~GLFrameBuffer()
{
	if (FramebufferTextureID)
		glDeleteTextures(1, &FramebufferTextureID);
	if (RenderbufferID)
		glDeleteRenderbuffers(1, &RenderbufferID);

	glDeleteFramebuffers(1, &FramebufferID);
}

void GLFrameBuffer::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferID);
}

void GLFrameBuffer::Unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GLFrameBuffer::CreateFrameBufferData()
{
	Bind();

	glGenTextures(1, &FramebufferTextureID);

	glBindTexture(GL_TEXTURE_2D, FramebufferTextureID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, renderDevice->Viewport->GetPixelWidth(), renderDevice->Viewport->GetPixelHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FramebufferTextureID, 0);

	glBindTexture(GL_TEXTURE_2D, 0);

	glGenRenderbuffers(1, &RenderbufferID);

	glBindRenderbuffer(GL_RENDERBUFFER, RenderbufferID);

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, renderDevice->Viewport->GetPixelWidth(), renderDevice->Viewport->GetPixelHeight());

	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	Unbind();
}

void GLFrameBuffer::UpdateFrameBufferData()
{
	Bind();

	glBindTexture(GL_TEXTURE_2D, FramebufferTextureID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, renderDevice->Viewport->GetPixelWidth(), renderDevice->Viewport->GetPixelHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

	glBindTexture(GL_TEXTURE_2D, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, RenderbufferID);

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, renderDevice->Viewport->GetPixelWidth(), renderDevice->Viewport->GetPixelHeight());

	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	Unbind();
}

std::vector<uint8_t> GLFrameBuffer::ReadPixelData()
{
	const size_t width = renderDevice->Viewport->GetPixelWidth();
	const size_t height = renderDevice->Viewport->GetPixelHeight();

	std::vector<uint8_t> result;

	result.resize(width * height * 4);

	Bind();

	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, result.data());

	Unbind();

	return result;
}
