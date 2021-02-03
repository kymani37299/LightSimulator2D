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

	ShaderInput* m_QuadInput;
	Shader* m_Shader;
	Texture* m_Texture;
};