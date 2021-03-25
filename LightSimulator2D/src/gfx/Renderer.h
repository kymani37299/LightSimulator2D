#pragma once

#include <vector>
#include <map>

class Window;

class Entity;
class Scene;

class LightingRenderer;
class LightOcclusionRenderer;
class AlbedoRenderer;
class SceneCuller;

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

	AlbedoRenderer* m_AlbedoRenderer;
	LightingRenderer* m_LightingRenderer;
	LightOcclusionRenderer* m_OcclusionRenderer;
	SceneCuller* m_SceneCuller;
};