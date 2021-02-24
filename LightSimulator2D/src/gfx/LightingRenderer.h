#pragma once

#include <vector>

class Framebuffer;
class Shader;

class Entity;

class LightingRenderer
{
public:
	LightingRenderer(Framebuffer* albedoFB, Framebuffer* occlusionFB);
	~LightingRenderer();

	void CompileShaders();

	void RenderLights();

	void OnEntityAdded(Entity& entity);
	void OnEntityRemoved(Entity& entity);

private:
	void RenderLighting();
	void RenderOccluders();

private:
	Framebuffer* m_AlbedoFB;
	Framebuffer* m_OcclusionFB;

	Shader* m_LightingShader = nullptr;
	Shader* m_AlbedoShader = nullptr;

	std::vector<Entity> m_Occluders;
	std::vector<Entity> m_Emitters;
};