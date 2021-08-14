#pragma once

#include "ui/UIElement.h"

class LightOcclusionUI : public UIElement
{
public:
	void Init() {}
	void Update(float dt);
	void Render();

private:
	bool m_DisableAngledRays = false;
	bool m_DebugIntersections = false;
	bool m_DebugRays = false;
	bool m_DebugMeshes = false;
	bool m_SimpleLightMask = false;
};