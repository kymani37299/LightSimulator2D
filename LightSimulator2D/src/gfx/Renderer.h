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
	std::vector<PointLight> m_PointLights;

	UniformBuffer* m_StaticTransformBuffer;
	unsigned m_NumStaticTransforms = 0;
};