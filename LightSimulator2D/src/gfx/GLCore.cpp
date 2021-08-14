#include "GLCore.h"

#include <glad/glad.h>

#include "util/FileUtil.h"
#include "util/StringUtil.h"
#include "util/ImageUtil.h"

#include <string>
#include <vector>
#include <set>

#include "glm/gtc/type_ptr.hpp"

#ifdef DEBUG

#include <iostream>

static void GLClearError()
{
	while (glGetError());
}

static bool GLCheckError()
{
	bool hasError = false;
	while (GLenum error = glGetError())
	{
		std::cout << "[OpenGL Error] " << error << std::endl;
		//LOG("[OpenGL Error] " + error);
		hasError = true;
	}
	return hasError;
}

#define GL_CALL(X) GLClearError();X;ASSERT(!GLCheckError())
#else
#define GL_CALL(X) X
#endif // DEBUG

// -------------------------------------------
// ---------- GLFunctions --------------------
// -------------------------------------------

ShaderInput* GLConstants::QuadInput = nullptr;

void GLFunctions::InitGL(void* procAddressGet)
{
	int glad_status = gladLoadGLLoader((GLADloadproc)procAddressGet);
	ASSERT(glad_status);

	static std::vector<Vertex> fc = {
		{Vec2(-1.0,-1.0)   ,Vec2(0.0,0.0)},
		{Vec2(1.0,-1.0)    ,Vec2(1.0,0.0)},
		{Vec2(-1.0,1.0)    ,Vec2(0.0,1.0)},

		{Vec2(-1.0,1.0)    ,Vec2(0.0,1.0)},
		{Vec2(1.0,-1.0)    ,Vec2(1.0,0.0)},
		{Vec2(1.0,1.0)     ,Vec2(1.0,1.0)}
	};
	// Init constants
	GLConstants::QuadInput = new ShaderInput(fc);
}

void GLFunctions::DeinitGL()
{
	delete GLConstants::QuadInput;
}

void GLFunctions::WaitForGpu()
{
	GL_CALL(glFinish());
}

void GLFunctions::Draw(unsigned numVertices)
{
	GL_CALL(glDrawArrays(GL_TRIANGLES, 0, numVertices));

#ifdef FORCE_BARRIERS
	GL_CALL(glFinish());
#endif
}

void GLFunctions::DrawPoints(unsigned numPoints)
{
	GL_CALL(glDrawArrays(GL_POINTS, 0, numPoints));
}

void GLFunctions::DrawFC()
{
	GLConstants::QuadInput->Bind();
	GL_CALL(glDrawArrays(GL_TRIANGLES, 0, 6));
}

void GLFunctions::Dispatch(unsigned groupX, unsigned groupY, unsigned groupZ)
{
	GL_CALL(glDispatchCompute(groupX, groupY, groupZ));
}

void GLFunctions::ClearScreen(Vec3 clearColor)
{
	GL_CALL(glClearColor(clearColor.r, clearColor.g, clearColor.b, 1));
	GL_CALL(glClear(GL_COLOR_BUFFER_BIT));
}

void GLFunctions::MemoryBarrier(BarrierType barrier)
{
	unsigned glBarrier = 0;
	switch (barrier)
	{
	case BarrierType::Image:
		glBarrier = GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;
		break;
	case BarrierType::VertexBuffer:
		glBarrier = GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT;
		break;
	case BarrierType::UniformBuffer:
		glBarrier = GL_UNIFORM_BARRIER_BIT;
		break;
	case BarrierType::BufferUpdate:
		glBarrier = GL_BUFFER_UPDATE_BARRIER_BIT;
		break;
	case BarrierType::ShaderStorage:
		glBarrier = GL_SHADER_STORAGE_BARRIER_BIT;
		break;
	case BarrierType::Framebuffer:
		glBarrier = GL_FRAMEBUFFER_BARRIER_BIT;
		break;
	case BarrierType::All:
		glBarrier = GL_ALL_BARRIER_BITS;
		break;
	default:
		ASSERT(0);
	}
	GL_CALL(glMemoryBarrier(glBarrier));
}

void GLFunctions::AlphaBlending(bool enabled)
{
	if (enabled)
	{
		GL_CALL(glEnable(GL_BLEND));
		GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	}
	else
	{
		GL_CALL(glDisable(GL_BLEND));
	}
}

// -------------------------------------------
// ---------- ShaderInput --------------------
// -------------------------------------------

void CreateShaderInput(GLHandle& vbo, GLHandle& vao, unsigned size, void* data)
{
	GL_CALL(glGenBuffers(1, &vbo));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
	GL_CALL(glGenVertexArrays(1, &vao));
	GL_CALL(glBindVertexArray(vao));
}

ShaderInput::ShaderInput(std::vector<Vertex> vertices)
{
	CreateShaderInput(m_VertexBuffer, m_VertexArray, vertices.size() * sizeof(Vertex), vertices.data());

	GL_CALL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0));
	GL_CALL(glEnableVertexAttribArray(0));
	GL_CALL(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(2*sizeof(float))));
	GL_CALL(glEnableVertexAttribArray(1));

	m_ElementNumber = vertices.size();
}

ShaderInput::ShaderInput(std::vector<Vec2> vertices)
{
	CreateShaderInput(m_VertexBuffer, m_VertexArray, vertices.size() * sizeof(Vec2), vertices.data());

	GL_CALL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2), (void*)0));
	GL_CALL(glEnableVertexAttribArray(0));

	m_ElementNumber = vertices.size();
}

ShaderInput::ShaderInput(GLHandle buffer):
	m_VertexBuffer(buffer),
	m_BufferOwner(false)
{
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer));
	GL_CALL(glGenVertexArrays(1, &m_VertexArray));
	GL_CALL(glBindVertexArray(m_VertexArray));

	// TOOD: Parametrize this
	GL_CALL(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vec4), (void*)0));
	GL_CALL(glEnableVertexAttribArray(0));
}

ShaderInput::~ShaderInput()
{
	if(m_BufferOwner)
		GL_CALL(glDeleteBuffers(1, &m_VertexBuffer));

	GL_CALL(glDeleteVertexArrays(1, &m_VertexArray));
}

void ShaderInput::Bind()
{
	GL_CALL(glBindVertexArray(m_VertexArray));
}

void ShaderInput::Unbind()
{
	GL_CALL(glBindVertexArray(0));
}

// -------------------------------------------
// ---------- UniformBuffer ------------------
// -------------------------------------------

UniformBuffer::UniformBuffer(unsigned stride, unsigned count)
{
	ASSERT(stride % 2 == 0); // Stride must be aligned to std140

	GL_CALL(glGenBuffers(1, &m_Handle));
	GL_CALL(glBindBuffer(GL_UNIFORM_BUFFER, m_Handle));
	GL_CALL(glBufferData(GL_UNIFORM_BUFFER, stride * count, NULL, GL_DYNAMIC_DRAW));
	GL_CALL(glBindBuffer(GL_UNIFORM_BUFFER, 0));

	m_Stride = stride;
	m_Count = count;
}

UniformBuffer::~UniformBuffer()
{
	GL_CALL(glDeleteBuffers(1, &m_Handle));
}

void UniformBuffer::Bind(unsigned slot)
{
	GL_CALL(glBindBufferBase(GL_UNIFORM_BUFFER, slot, m_Handle));
	m_CurrentSlot = slot;
}

void UniformBuffer::Unbind()
{
	if (m_CurrentSlot != -1)
	{
		GL_CALL(glBindBufferBase(GL_UNIFORM_BUFFER, m_CurrentSlot, 0));
		m_CurrentSlot = -1;
	}
}

void* UniformBuffer::Map()
{
	GL_CALL(glBindBuffer(GL_UNIFORM_BUFFER, m_Handle));
	GL_CALL(void* ptr = glMapBuffer(GL_UNIFORM_BUFFER, GL_READ_ONLY));
	return ptr;
}

void UniformBuffer::Unmap()
{
	GL_CALL(glUnmapBuffer(GL_UNIFORM_BUFFER));
	GL_CALL(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}

void UniformBuffer::UploadData(void* data, unsigned index, unsigned count)
{
	ASSERT(index + count <= m_Count);

	GL_CALL(glBindBuffer(GL_UNIFORM_BUFFER, m_Handle));
	GL_CALL(glBufferSubData(GL_UNIFORM_BUFFER, index * m_Stride, count * m_Stride, data));
	GL_CALL(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}

// -------------------------------------------
// ---------- ShaderStorageBuffer ------------
// -------------------------------------------

ShaderStorageBuffer::ShaderStorageBuffer(unsigned stride, unsigned count):
	m_Stride(stride),
	m_Count(count)
{
	GL_CALL(glGenBuffers(1, &m_Handle));
	GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_Handle));
	GL_CALL(glBufferData(GL_SHADER_STORAGE_BUFFER, stride*count, NULL, GL_DYNAMIC_COPY));
	GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
}

ShaderStorageBuffer::~ShaderStorageBuffer()
{
	GL_CALL(glDeleteBuffers(1, &m_Handle));
}

ShaderInput* ShaderStorageBuffer::AsShaderInput()
{
	ShaderInput* shaderInput = new ShaderInput(m_Handle);
	return shaderInput;
}

void ShaderStorageBuffer::Bind(unsigned slot)
{
	GL_CALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, slot, m_Handle));
	m_CurrentSlot = slot;
}

void ShaderStorageBuffer::Unbind()
{
	if (m_CurrentSlot != -1)
	{
		GL_CALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_CurrentSlot, 0));
		m_CurrentSlot = -1;
	}
}

void* ShaderStorageBuffer::Map(bool write)
{
	GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_Handle));
	GL_CALL(void* ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, write ? GL_READ_WRITE : GL_READ_ONLY));
	return ptr;
}

void ShaderStorageBuffer::Unmap()
{
	GL_CALL(glUnmapBuffer(GL_SHADER_STORAGE_BUFFER));
	GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
}

void ShaderStorageBuffer::UploadData(void* data, unsigned index, unsigned count)
{
	ASSERT(index + count <= m_Count);

	GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_Handle));
	GL_CALL(glBufferSubData(GL_SHADER_STORAGE_BUFFER, index * m_Stride, count * m_Stride, data));
	GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
}

// -------------------------------------------
// ---------- Texture ------------------------
// -------------------------------------------

Texture::Texture(const std::string& path)
{
	ImageData* imageData = ImageUtil::LoadImage(path.c_str(), true);

	ASSERT(imageData->data);

	m_Width = imageData->width;
	m_Height = imageData->height;

	GL_CALL(glGenTextures(1, &m_Handle));
	GL_CALL(glActiveTexture(GL_TEXTURE0));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, m_Handle));

	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData->data));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));

	ImageUtil::FreeImage(imageData->data);
}

Texture::~Texture()
{
	GL_CALL(glDeleteTextures(1, &m_Handle));
}

void Texture::Bind(unsigned slot)
{
	GL_CALL(glActiveTexture(GL_TEXTURE0 + slot));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, m_Handle));

	m_CurrentSlot = slot;
}

void Texture::Unbind()
{
	if (m_CurrentSlot != -1)
	{
		GL_CALL(glActiveTexture(GL_TEXTURE0 + m_CurrentSlot));
		GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
		m_CurrentSlot = -1;
	}

}

void Texture::SetRepeatedScaling(bool value)
{
	GLenum scaling = value ? GL_REPEAT : GL_CLAMP_TO_EDGE;

	GL_CALL(glActiveTexture(GL_TEXTURE0));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, m_Handle));

	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, scaling));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, scaling));

	GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
}

// -------------------------------------------
// ---------- Image --------------------------
// -------------------------------------------

Image::Image(unsigned width, unsigned height, unsigned flags):
	m_Width(width),
	m_Height(height),
	m_Flags(flags)
{
	GL_CALL(glGenTextures(1, &m_Handle));
	GL_CALL(glActiveTexture(GL_TEXTURE0));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, m_Handle));

	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));

	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_Width, m_Height, 0, GL_RGBA, GL_FLOAT, NULL));

	unsigned accessFlags = GL_READ_ONLY;
	if (flags | IF_ReadAccess && flags | IF_WriteAccess)
		accessFlags = GL_READ_WRITE;
	else if (flags | IF_WriteAccess)
		accessFlags = GL_WRITE_ONLY;

	GL_CALL(glBindImageTexture(0, m_Handle, 0, GL_FALSE, 0, accessFlags, GL_RGBA32F));
}

Image::~Image()
{
	GL_CALL(glDeleteTextures(1, &m_Handle));
}

void Image::Bind(unsigned slot)
{
	GL_CALL(glActiveTexture(GL_TEXTURE0 + slot));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, m_Handle));
	m_CurrentSlot = slot;
}

void Image::Unbind()
{
	if (m_CurrentSlot != -1)
	{
		GL_CALL(glActiveTexture(GL_TEXTURE0 + m_CurrentSlot));
		GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
		m_CurrentSlot = -1;
	}
}

// -------------------------------------------
// ---------- Shader -------------------------
// -------------------------------------------

std::string GetTag(GLenum type)
{
	switch (type)
	{
	case GL_VERTEX_SHADER:
		return "[VS]";
	case GL_FRAGMENT_SHADER:
		return "[FS]";
	case GL_COMPUTE_SHADER:
		return "[CS]";
	case GL_GEOMETRY_SHADER:
		return "[GS]";
	default:
		return "[Unknown shader type]";
	}
}

static GLHandle CompileShader(uint32_t type, const char* source)
{
	GL_CALL(GLHandle id = glCreateShader(type));
	GL_CALL(glShaderSource(id, 1, &source, nullptr));
	GL_CALL(glCompileShader(id));

	int result;
	GL_CALL(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
	if (!result)
	{
		int length;
		GL_CALL(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
		char* message = (char*)alloca(length * sizeof(char));
		GL_CALL(glGetShaderInfoLog(id, length, &length, message));
		LOG(GetTag(type) + " " + message);
		GL_CALL(glDeleteShader(id));
		return 0;
	}

	return id;
}

GLenum MacroToShaderType(const std::string macro)
{
	if (macro.find("#start VERTEX") != std::string::npos) return GL_VERTEX_SHADER;
	if (macro.find("#start FRAGMENT") != std::string::npos) return GL_FRAGMENT_SHADER;
	if (macro.find("#start COMPUTE") != std::string::npos) return GL_COMPUTE_SHADER;
	if (macro.find("#start GEOMETRY") != std::string::npos) return GL_GEOMETRY_SHADER;
	return 0;
}

static bool LoadAndCompileShader(std::string path, GLHandle& shaderHandle)
{
	std::string code = "";
	GLenum type = 0;
	path = SHADER_PATH + path;

	std::vector<std::string> shaderContent;

	if (!FileUtil::ReadFile(path, shaderContent))
	{
		LOG("[SHADER_LOAD] Failed to load a file: " + path);
		return false;
	}

	std::set<std::string> loadedFiles = {};

	std::vector<GLHandle> shaderModules;
	for (size_t i = 0; i < shaderContent.size(); i++)
	{
		std::string& line = shaderContent[i];

		GLenum next_type = MacroToShaderType(line);
		if (next_type != 0)
		{
			if (!code.empty())
			{
				GLHandle shaderModule = CompileShader(type, code.c_str());
				if (shaderModule == 0) return false;
				shaderModules.push_back(shaderModule);
			}
			code = "#version 430\n";
			type = next_type;
			continue;
		}

		if (type == 0) continue; // TODO: Add common code

		if (line.find("#include") != std::string::npos)
		{
			std::string fileName = line;
			StringUtil::ReplaceAll(fileName, "#include", "");
			StringUtil::ReplaceAll(fileName, " ", "");
			StringUtil::ReplaceAll(fileName, "\"", "");

			if (loadedFiles.count(fileName)) continue;
			loadedFiles.insert(fileName);

			std::vector<std::string> _c;
			if (!FileUtil::ReadFile(SHADER_PATH + fileName, _c))
			{
				LOG("[SHADER_LOAD] Failed to include " + fileName + " in " + path);
			}
			shaderContent.insert((shaderContent.begin() + (i + 1)), _c.begin(), _c.end());
		}
		else
		{
			code.append(line + "\n");
		}
	}

	GLHandle shaderModule = CompileShader(type, code.c_str());
	if (shaderModule == 0) return false;
	shaderModules.push_back(shaderModule);

	if (shaderModules.size() == 0) return false;

	for (GLHandle sm : shaderModules) { GL_CALL(glAttachShader(shaderHandle, sm)); }
	GL_CALL(glLinkProgram(shaderHandle));

	GLint validLinking;
	GL_CALL(glGetProgramiv(shaderHandle, GL_LINK_STATUS, (int*)&validLinking));
	GL_CALL(glValidateProgram(shaderHandle));

#ifdef DEBUG
	for (GLHandle sm : shaderModules) { GL_CALL(glDetachShader(shaderHandle, sm)); }
#else
	for (GLHandle sm : shaderModules) { GL_CALL(glDeleteShader(sm)); }
#endif

	return validLinking;
}

Shader::Shader(const std::string path)
{
	GL_CALL(m_Handle = glCreateProgram());

	m_Valid = LoadAndCompileShader(path, m_Handle);
}

Shader::~Shader()
{
	GL_CALL(glDeleteProgram(m_Handle));
}

void Shader::Bind()
{
	GL_CALL(glUseProgram(m_Handle));
}

void Shader::Unbind()
{
	GL_CALL(glUseProgram(0));
}

void Shader::SetUniformBlock(const std::string& block_name, unsigned int slot) const
{
	GL_CALL(unsigned int index = glGetUniformBlockIndex(m_Handle, block_name.c_str()));
	if (index != GL_INVALID_INDEX)
	{
		GL_CALL(glUniformBlockBinding(m_Handle, index, slot));
	}
}

static int GetUniformLocation(GLHandle shader, const std::string& name)
{
	GL_CALL(int location = glGetUniformLocation(shader, name.c_str()));
	return location;
}

template<> void Shader::SetUniform<bool>(const std::string& key, bool value) const
{
	GL_CALL(glUniform1i(GetUniformLocation(m_Handle, key), value));
}

template<> void Shader::SetUniform<int>(const std::string& key, int value) const
{
	GL_CALL(glUniform1i(GetUniformLocation(m_Handle, key), value));
}

template<> void Shader::SetUniform<float>(const std::string& key, float value) const
{
	GL_CALL(glUniform1f(GetUniformLocation(m_Handle, key), value));
}

template<> void Shader::SetUniform<Vec2>(const std::string& key, Vec2 data) const
{
	GL_CALL(glUniform2fv(GetUniformLocation(m_Handle, key), 1, glm::value_ptr(data)));
}

template<> void Shader::SetUniform<Vec3>(const std::string& key, Vec3 data) const
{
	GL_CALL(glUniform3fv(GetUniformLocation(m_Handle, key), 1, glm::value_ptr(data)));
}

template<> void Shader::SetUniform<Mat3>(const std::string& key, Mat3 value) const
{
	GL_CALL(glUniformMatrix3fv(GetUniformLocation(m_Handle, key), 1, GL_FALSE, glm::value_ptr(value)));
}

// -------------------------------------------
// ---------- Framebuffer --------------------
// -------------------------------------------

Framebuffer::Framebuffer(unsigned width, unsigned height, unsigned numColorAttachments):
	m_Width(width),
	m_Height(height),
	m_NumColorAttachments(numColorAttachments)
{
	ASSERT(m_NumColorAttachments <= MAX_COLOR_ATTACHMENTS);

	GL_CALL(glGenFramebuffers(1, &m_Handle));
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, m_Handle));

	for (unsigned i = 0; i < m_NumColorAttachments; i++)
	{
		GL_CALL(glGenTextures(1, &m_ColorAttachments[i]));
		GL_CALL(glBindTexture(GL_TEXTURE_2D, m_ColorAttachments[i]));

		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

		GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
		GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));

		GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_ColorAttachments[i], 0));
	}

}

Framebuffer::~Framebuffer()
{
	for (unsigned i = 0; i < m_NumColorAttachments; i++)
	{
		GL_CALL(glDeleteTextures(1, &m_ColorAttachments[i]));
	}
	GL_CALL(glDeleteFramebuffers(1, &m_Handle));
}

void Framebuffer::ClearAndBind()
{
	Bind();
	GLFunctions::ClearScreen();
}

void Framebuffer::Clear()
{
	Bind();
	GLFunctions::ClearScreen();
	Unbind();
}

void Framebuffer::Bind()
{
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, m_Handle));
}

void Framebuffer::Unbind()
{
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void Framebuffer::BindTexture(unsigned colorAttachment, unsigned slot)
{
	GL_CALL(glActiveTexture(GL_TEXTURE0 + slot));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, m_ColorAttachments[colorAttachment]));
	m_CurrentSlots[colorAttachment] = slot;
}

void Framebuffer::UnbindTexture(unsigned colorAttachment)
{
	if (m_CurrentSlots[colorAttachment] != -1)
	{
		GL_CALL(glActiveTexture(GL_TEXTURE0 + m_CurrentSlots[colorAttachment]));
		GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
		m_CurrentSlots[colorAttachment] = -1;
	}
}
