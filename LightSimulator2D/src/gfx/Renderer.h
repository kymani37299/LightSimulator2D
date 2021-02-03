#pragma once

class Window;

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
};