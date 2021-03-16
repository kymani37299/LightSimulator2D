#pragma once

#include <vector>

class Framebuffer;
class Shader;

class Scene;

class LightingRenderer
{
public:
	LightingRenderer(Framebuffer* albedoFB, Framebuffer* occlusionFB);
	~LightingRenderer();

	void CompileShaders();

	void RenderLighting(Scene* scene);
	void RenderEmitters(Scene* scene);

private:
	Framebuffer* m_AlbedoFB;
	Framebuffer* m_OcclusionFB;

	Shader* m_LightingShader = nullptr;
	Shader* m_EmitterShader = nullptr;
	Shader* m_OccluderShader = nullptr;
};