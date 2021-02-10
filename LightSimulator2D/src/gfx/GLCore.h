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

};

namespace GLFunctions
{
	void InitGL(void* procAddressGet);
	void Draw(unsigned numVertices);
	void Dispatch(unsigned groupX = 1, unsigned groupY = 1, unsigned groupZ = 1);
	void ClearScreen(Vec3 clearColor = VEC3_ZERO);
	void MemoryBarrier(BarrierType barrier);
};

// TODO: Delete copy constructors from all gl classes

class ShaderInput
{
public:
	ShaderInput(std::vector<Vertex> vertices);
	~ShaderInput();

	void Bind();
	void Unbind();

private:
	GLHandle m_VertexBuffer;
	GLHandle m_VertexArray;
};

class UniformBuffer
{
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
public:
	ShaderStorageBuffer(unsigned stride, unsigned count = 1);
	~ShaderStorageBuffer();

	void Bind(unsigned slot);
	void Unbind();

private:
	GLHandle m_Handle;

	int m_CurrentSlot = -1;
	unsigned m_Stride;
	unsigned m_Count;

};

class Texture
{
public:
	Texture(const std::string& path);
	~Texture();

	void Bind(unsigned slot);
	void Unbind();

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
public:
	Shader(const std::string& vert, const std::string& frag);
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

class ComputeShader
{
	static bool s_InitializedHW;
	static void InitializeHW();

	static int s_MaxGroupSize[3];
	static int s_MaxGroupInvocations;
	
public:
	ComputeShader(const std::string& cs);
	~ComputeShader();

	void Bind();
	void Unbind();

	inline bool IsValid() const { return m_Valid; }

	template<typename T> void SetUniform(const std::string& key, T value) const;

private:
	GLHandle m_Handle;
	bool m_Valid = true;
};

class Framebuffer
{
	static constexpr int MAX_COLOR_ATTACHMENTS = 5;
public:
	Framebuffer(unsigned width, unsigned height, unsigned numColorAttachments = 1);
	~Framebuffer();

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