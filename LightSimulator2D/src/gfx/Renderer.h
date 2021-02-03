#pragma once

#include <vector>

class Window;
class Shader;
class ShaderInput;
class Texture;
class Scene;

class Renderer
{
public:
	~Renderer();

	void Init(Window& window);
	void Update(float dt);
	bool RenderIfNeeded();

	inline void SetScene(Scene* scene) { m_Scene = scene; }

private:
	void RenderFrame();

private:
	bool m_ShouldRender = true;

	ShaderInput* m_QuadInput;
	Shader* m_Shader;

	Scene* m_Scene;

	std::vector<Texture*> m_Textures;
};