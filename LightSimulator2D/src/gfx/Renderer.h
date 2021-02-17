#pragma once

#include <vector>
#include <map>

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

	void OnEntityAdded(Entity& e);
	void OnEntityRemoved(Entity& e);

	void ReloadShaders() { m_ShouldReloadShaders = true; }

private:
	void CompileShaders();
	void InitEntityForRender(Entity& e);
	void RemoveEntityFromRenderPipeline(Entity& e);

	void RenderFrame();

private:
	bool m_ShouldReloadShaders = false;
	bool m_ShouldRender = true;
	Scene* m_Scene;

	Framebuffer* m_AlbedoFB;
	Framebuffer* m_OcclusionMaskFB;

	ShaderInput* m_QuadInput;

	Shader* m_OpaqueShader;
	Shader* m_ShadowmapShader;
	Shader* m_LightingShader;

	LightOcclusionRenderer* m_OcclusionRenderer;
};