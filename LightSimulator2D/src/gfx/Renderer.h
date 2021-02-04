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

class Renderer
{
public:
	~Renderer();

	void Init(Window& window);
	void Update(float dt);
	bool RenderIfNeeded();

	void SetScene(Scene* scene);

	void OnEntityAdded(Entity& e);
	void OnEntityRemoved(Entity& e);

private:
	void InitEntityForRender(Entity& e);

	void RenderFrame();

private:
	bool m_ShouldRender = true;

	ShaderInput* m_QuadInput;
	Shader* m_Shader;

	Scene* m_Scene;

	std::vector<Texture*> m_Textures;

	std::vector<PointLight*> m_PointLights;
};