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

static void GLClearError()
{
	while (glGetError());
}

static bool GLCheckError()
{
	bool hasError = false;
	while (GLenum error = glGetError())
	{
		LOG("[OpenGL Error] " + error);
		hasError = true;
	}
	return hasError;
}

#define GL_CALL(X) GLClearError();X;ASSERT(!GLCheckError())
#else
#define GL_CALL(X) X
#endif // DEBUG

static bool LoadShader(std::string path, std::string& shaderCode)
{
	shaderCode = "";
	path = SHADER_PATH + path;

	std::vector<std::string> shaderContent;
	if (!FileUtil::ReadFile(path, shaderContent))
	{
		LOG("[SHADER_LOAD] Failed to load a file: " + path);
		return false;
	}

	std::set<std::string> loadedFiles = {};
	for (size_t i = 0; i < shaderContent.size(); i++)
	{
		std::string& line = shaderContent[i];
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
			shaderCode.append(line + "\n");
		}
	}
	return true;
}

static uint32_t CompileShader(uint32_t type, const char* source)
{
	GL_CALL(uint32_t id = glCreateShader(type));
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
		std::string error_tag = type == GL_VERTEX_SHADER ? "[VS]" : "[FS]";
		LOG(error_tag + " " + message);
		GL_CALL(glDeleteShader(id));
		return 0;
	}

	return id;
}

// -------------------------------------------
// ---------- GLFunctions --------------------
// -------------------------------------------

void GLFunctions::InitGL(void* procAddressGet)
{
	int glad_status = gladLoadGLLoader((GLADloadproc)procAddressGet);
	ASSERT(glad_status);
}

void GLFunctions::Draw(unsigned numVertices)
{
	GL_CALL(glDrawArrays(GL_TRIANGLES, 0, numVertices));
}

void GLFunctions::ClearScreen(Vec3 clearColor)
{
	GL_CALL(glClearColor(clearColor.r, clearColor.g, clearColor.b, 1));
	GL_CALL(glClear(GL_COLOR_BUFFER_BIT));
}

// -------------------------------------------
// ---------- ShaderInput --------------------
// -------------------------------------------

ShaderInput::ShaderInput(std::vector<Vertex> vertices)
{
	GL_CALL(glGenBuffers(1, &m_VertexBuffer));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW));

	GL_CALL(glGenVertexArrays(1, &m_VertexArray));
	GL_CALL(glBindVertexArray(m_VertexArray));

	GL_CALL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0));
	GL_CALL(glEnableVertexAttribArray(0));
	GL_CALL(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(2*sizeof(float))));
	GL_CALL(glEnableVertexAttribArray(1));
}

ShaderInput::~ShaderInput()
{
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
// ---------- Texture ------------------------
// -------------------------------------------

Texture::Texture(const std::string& path)
{
	ImageData* imageData = ImageUtil::LoadImage(path.c_str(), true);

	m_Width = imageData->width;
	m_Height = imageData->height;

	GL_CALL(glGenTextures(1, &m_Handle));
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
}

void Texture::Unbind(unsigned slot)
{
	GL_CALL(glActiveTexture(GL_TEXTURE0 + slot));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
}

// -------------------------------------------
// ---------- Shader -------------------------
// -------------------------------------------

Shader::Shader(const std::string& vert, const std::string& frag)
{
	GL_CALL(m_Handle = glCreateProgram());

	std::string vertexCode;
	std::string fragmentCode;

	if (!LoadShader(vert, vertexCode) || !LoadShader(frag, fragmentCode))
	{
		m_Valid = false;
		return;
	}

	unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexCode.c_str());
	unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentCode.c_str());

	if (!vs || !fs)
	{
		m_Valid = false;
		return;
	}

	GL_CALL(glAttachShader(m_Handle, vs));
	GL_CALL(glAttachShader(m_Handle, fs));
	// TODO: Validate linking
	GL_CALL(glLinkProgram(m_Handle));
	GL_CALL(glValidateProgram(m_Handle));

#ifdef DEBUG
	GL_CALL(glDetachShader(m_Handle, vs));
	GL_CALL(glDetachShader(m_Handle, fs));
#else
	GL_CALL(glDeleteShader(vs));
	GL_CALL(glDeleteShader(fs));
#endif
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

static int GetUniformLocation(GLHandle shader, const std::string& name)
{
	GL_CALL(int location = glGetUniformLocation(shader, name.c_str()));
	return location;
}

template<> void Shader::SetUniform<int>(const std::string& key, int value) const
{
	GL_CALL(glUniform1i(GetUniformLocation(m_Handle, key), value));
}

template<> void Shader::SetUniform<Mat3>(const std::string& key, Mat3 value) const
{
	GL_CALL(glUniformMatrix3fv(GetUniformLocation(m_Handle, key), 1, GL_FALSE, glm::value_ptr(value)));
}

