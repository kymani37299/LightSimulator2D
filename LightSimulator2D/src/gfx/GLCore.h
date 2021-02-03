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

namespace GLFunctions
{
	void InitGL(void* procAddressGet);
	void Draw(unsigned numVertices);
	void ClearScreen(Vec3 clearColor = VEC3_ZERO);
};

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

class Texture
{
public:
	Texture(const std::string& path);
	~Texture();

	void Bind(unsigned slot);
	void Unbind(unsigned slot);

	inline unsigned GetWidth() const { return m_Width; }
	inline unsigned GetHeight() const { return m_Height; }

private:
	GLHandle m_Handle;
	
	unsigned m_Width;
	unsigned m_Height;
};

class Shader
{
public:
	Shader(const std::string& vert, const std::string& frag);
	~Shader();

	void Bind();
	void Unbind();

	inline bool IsValid() const { return m_Valid; }

	template<typename T> void SetUniform(const std::string& key, T value) const;

private:
	GLHandle m_Handle;
	bool m_Valid = true;
};