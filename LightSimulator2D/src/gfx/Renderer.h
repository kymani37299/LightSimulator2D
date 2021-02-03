#pragma once

class Window;
class Shader;
class ShaderInput;

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
};