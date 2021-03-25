#pragma once

#include <vector>

class Framebuffer;
class Shader;
class CulledScene;

class LightingRenderer
{
public:
	LightingRenderer(Framebuffer* albedoFB, Framebuffer* occlusionFB);
	~LightingRenderer();

	void CompileShaders();

	void RenderLighting(CulledScene& scene);
	void RenderEmitters(CulledScene& scene);

private:
	Framebuffer* m_AlbedoFB;
	Framebuffer* m_OcclusionFB;

	Shader* m_LightingShader = nullptr;
	Shader* m_EmitterShader = nullptr;
	Shader* m_OccluderShader = nullptr;
};