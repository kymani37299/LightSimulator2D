#pragma once

#include <vector>
#include <string>

#include "common.h"

struct Vertex
{
	// Note: When changing this update VAO from ShaderInput
	Vec2 position;
	Vec2 uv;
};

using GLHandle = unsigned int;

enum class BarrierType
{
	Image,
	VertexBuffer,
	UniformBuffer,
	BufferUpdate,
	ShaderStorage,
	Framebuffer,
	All

};

namespace GLFunctions
{
	void InitGL(void* procAddressGet);
	void DeinitGL();
	void Draw(unsigned numVertices);
	void DrawPoints(unsigned numPoints);
	void DrawFC();
	void Dispatch(unsigned groupX = 1, unsigned groupY = 1, unsigned groupZ = 1);
	void ClearScreen(Vec3 clearColor = VEC3_ZERO);
	void MemoryBarrier(BarrierType barrier);
	void AlphaBlending(bool enabled);
};

class ShaderInput
{
	DELETE_COPY_CONSTRUCTOR(ShaderInput);
public:
	ShaderInput(std::vector<Vertex> vertices);
	ShaderInput(std::vector<Vec2> vertices);
	ShaderInput(GLHandle buffer);

	~ShaderInput();

	inline unsigned GetElementNumber() const { return m_ElementNumber; }

	void Bind();
	void Unbind();

private:
	unsigned m_ElementNumber = 0;

	bool m_BufferOwner = true;
	GLHandle m_VertexBuffer;
	GLHandle m_VertexArray;
};

class UniformBuffer
{
	DELETE_COPY_CONSTRUCTOR(UniformBuffer);
public:
	UniformBuffer(unsigned stride, unsigned count = 1);
	~UniformBuffer();

	void Bind(unsigned slot);
	void Unbind();

	void UploadData(void* data, unsigned index = 0, unsigned count = 1);

private:
	GLHandle m_Handle;

	int m_CurrentSlot = -1;
	unsigned m_Stride;
	unsigned m_Count;
};

class ShaderStorageBuffer
{
	DELETE_COPY_CONSTRUCTOR(ShaderStorageBuffer);
public:
	ShaderStorageBuffer(unsigned stride, unsigned count = 1);
	~ShaderStorageBuffer();

	ShaderInput* AsShaderInput();

	void Bind(unsigned slot);
	void Unbind();

	void* Map();
	void Unmap();

private:
	GLHandle m_Handle;

	int m_CurrentSlot = -1;
	unsigned m_Stride;
	unsigned m_Count;

};

class Texture
{
	DELETE_COPY_CONSTRUCTOR(Texture);
public:
	Texture(const std::string& path);
	~Texture();

	void Bind(unsigned slot);
	void Unbind();

	void SetRepeatedScaling(bool value);

	inline unsigned GetWidth() const { return m_Width; }
	inline unsigned GetHeight() const { return m_Height; }

private:
	GLHandle m_Handle;
	
	int m_CurrentSlot = -1;
	unsigned m_Width;
	unsigned m_Height;

};

enum ImageFlags
{
	IF_WriteAccess = 1,
	IF_ReadAccess = 2
	// Next 4
};

class Image
{
	DELETE_COPY_CONSTRUCTOR(Image);
public:
	Image(unsigned width, unsigned height, unsigned flags);
	~Image();

	void Bind(unsigned slot);
	void Unbind();

private:
	GLHandle m_Handle;

	int m_CurrentSlot = -1;
	unsigned m_Width;
	unsigned m_Height;
	unsigned m_Flags;
};

class Shader
{
	DELETE_COPY_CONSTRUCTOR(Shader);
public:
	Shader(const std::string path);
	~Shader();

	void Bind();
	void Unbind();

	inline bool IsValid() const { return m_Valid; }

	void SetUniformBlock(const std::string& block_name, unsigned int slot) const;
	template<typename T> void SetUniform(const std::string& key, T value) const;

private:
	GLHandle m_Handle;
	bool m_Valid = true;
};

class Framebuffer
{
	DELETE_COPY_CONSTRUCTOR(Framebuffer);

	static constexpr int MAX_COLOR_ATTACHMENTS = 5;
public:
	Framebuffer(unsigned width, unsigned height, unsigned numColorAttachments = 1);
	~Framebuffer();

	void ClearAndBind();
	void Clear();
	void Bind();
	void Unbind();

	void BindTexture(unsigned colorAttachment, unsigned slot);
	void UnbindTexture(unsigned colorAttachment);

private:
	GLHandle m_Handle;
	GLHandle m_ColorAttachments[MAX_COLOR_ATTACHMENTS] = { 0 };

	int m_CurrentSlots[MAX_COLOR_ATTACHMENTS] = { -1 };
	unsigned m_NumColorAttachments;
	unsigned m_Width;
	unsigned m_Height;
};

namespace GLConstants
{
	extern ShaderInput* QuadInput;
};