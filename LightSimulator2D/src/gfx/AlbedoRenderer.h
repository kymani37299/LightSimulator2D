#pragma once

class Framebuffer;
class Shader;

class Scene;
class Entity;

class AlbedoRenderer
{
public:
	AlbedoRenderer();
	~AlbedoRenderer();

	void CompileShaders();

	void RenderBackground(Scene* scene);
	void RenderBase(Scene* scene);
	void RenderOccluders(Scene* scene);
	void RenderForeground(Scene* scene);

	inline Framebuffer* GetAlbedoFB() const { return m_AlbedoFB; }

private:
	void RenderEntity(Entity* entity);
	void SetupLightSources(Scene* scene, bool ignoreCam = true);

private:
	Framebuffer* m_AlbedoFB;
	Shader* m_AlbedoShader = nullptr;
};