#pragma once

#include <vector>
#include <map>

class Window;

class Entity;
class Scene;

class Shader;
class Framebuffer;

class LightingRenderer;
class LightOcclusionRenderer;

class Renderer
{
public:
	~Renderer();

	void Init(Window& window);
	void Update(float dt);
	bool RenderIfNeeded();

	void SetScene(Scene* scene);
	void FreeScene();

	void OnEntityAdded(Entity* e);
	void OnEntityRemoved(Entity* e);

	void ReloadShaders() { m_ShouldReloadShaders = true; }

private:
	void CompileShaders();
	void InitEntityForRender(Entity* e);
	void RemoveEntityFromRenderPipeline(Entity* e);

	void RenderFrame();

private:
	bool m_ShouldReloadShaders = false;
	bool m_ShouldRender = true;
	Scene* m_Scene;

	Framebuffer* m_AlbedoFB;

	Shader* m_AlbedoShader;

	LightingRenderer* m_LightingRenderer;
	LightOcclusionRenderer* m_OcclusionRenderer;
};