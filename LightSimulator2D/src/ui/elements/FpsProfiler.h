#pragma once

#include <vector>
#include "ui/UIElement.h"

class Window;

class FpsProfiler : public UIElement
{
public:
	FpsProfiler();

	void Init() {}
	void Update(float dt);
	void Render();

private:
	float m_CurrentFps = 0.0f;
	float m_FpsAge = 0.0f;

	std::vector<float> m_FpsValues;
	Window* m_Window = nullptr;
};