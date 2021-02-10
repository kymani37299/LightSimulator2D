#pragma once

#include <vector>
#include <map>

#include "gfx/Lights.h"

class Window;
class Shader;
class ShaderInput;
class Texture;
class Scene;
class Entity;
class ComputeShader;
class Image;
class Framebuffer;
class UniformBuffer;
class ShaderStorageBuffer;

class Renderer
{
public:
	~Renderer();

	void Init(Window& window);
	void Update(float dt);
	bool RenderIfNeeded();

	void SetScene(Scene* scene);
	void FreeScene();

	void OnEntityAdded(Entity& e);
	void OnEntityRemoved(Entity& e);

private:
	void InitEntityForRender(Entity& e);
	void RemoveEntityFromRenderPipeline(Entity& e);

	void RenderFrame();

private:
	bool m_ShouldRender = true;
	Scene* m_Scene;

	ShaderInput* m_QuadInput;

	ComputeShader* m_LightOcclusionShader;
	Shader* m_OpaqueShader;
	Shader* m_ShadowmapShader;

	static constexpr unsigned NUM_INTERSECTIONS = 30;
	ShaderStorageBuffer* m_IntersectionBuffer;
	ShaderInput* m_IntersectionShaderInput;

	UniformBuffer* m_OcclusionLines;
	unsigned m_OcclusionLineCount = 0;
};