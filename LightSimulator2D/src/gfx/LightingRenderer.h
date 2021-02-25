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

	void RenderLights(Scene* scene);

private:
	void RenderLighting();
	void RenderOccluders(Scene* scene);

private:
	Framebuffer* m_AlbedoFB;
	Framebuffer* m_OcclusionFB;

	Shader* m_LightingShader = nullptr;
	Shader* m_AlbedoShader = nullptr;
};