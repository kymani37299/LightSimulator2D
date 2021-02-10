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

	ShaderInput* m_QuadInput;
	Shader* m_Shader;

	Scene* m_Scene;

	static constexpr unsigned NUM_INTERSECTIONS = 30;
	ShaderStorageBuffer* m_IntersectionBuffer;

	UniformBuffer* m_OcclusionLines;
	unsigned m_OcclusionLineCount = 0;

	ComputeShader* m_LightOcclusionShader;
};