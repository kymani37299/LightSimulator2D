#pragma once

class Framebuffer;
class Shader;

class CulledScene;
class CulledEntity;

class AlbedoRenderer
{
public:
	AlbedoRenderer();
	~AlbedoRenderer();

	void CompileShaders();

	void RenderBackground(CulledScene& scene);
	void RenderBase(CulledScene& scene);
	void RenderOccluders(CulledScene& scene);
	void RenderForeground(CulledScene& scene);

	inline Framebuffer* GetAlbedoFB() const { return m_AlbedoFB; }

private:
	void RenderEntity(CulledEntity* entity);
	void SetupLightSources(CulledScene& scene, bool ignoreCam = true);

	void SetupDefaultParams(CulledScene& scene);

private:
	Framebuffer* m_AlbedoFB;
	Shader* m_AlbedoShader = nullptr;
};