#pragma once

class Window;
class Shader;
class ShaderInput;
class Texture;

class Renderer
{
public:
	~Renderer();

	void Init(Window& window);
	void Update(float dt);
	bool RenderIfNeeded();

private:
	void RenderFrame();

private:
	bool m_ShouldRender = true;

	Shader* m_Shader;
	ShaderInput* m_Triangle;
	Texture* m_Texture;
};