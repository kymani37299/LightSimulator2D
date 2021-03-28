#pragma once

#include "ui/UIElement.h"

class SceneUI : public UIElement
{
	static SceneUI* s_Instance;

public:
	static SceneUI* Get()
	{
		if (!s_Instance) s_Instance = new SceneUI();
		return s_Instance;
	}

	void Init() {}
	void Update(float) {}
	void Render();

	inline void SetTotalEntities(unsigned value) { m_TotalEntities = value; }
	inline void SetTotalInstances(unsigned value) { m_TotalInstances = value; }
	inline void SetDrawnInstances(unsigned value) { m_DrawnInstances = value; }
	inline void SetTotalOccluders(unsigned value) { m_TotalOccluders = value; }
	inline void SetDrawnOccluders(unsigned value) { m_DrawnOccluders = value; }
	inline void SetTotalEmitters(unsigned value) { m_TotalEmitters = value; }
	inline void SetDrawnEmitters(unsigned value) { m_DrawnEmitters = value; }

private:
	SceneUI() {}

	unsigned m_TotalEntities = 0;

	unsigned m_TotalInstances = 0;
	unsigned m_DrawnInstances = 0;

	unsigned m_TotalOccluders = 0;
	unsigned m_DrawnOccluders = 0;

	unsigned m_TotalEmitters = 0;
	unsigned m_DrawnEmitters = 0;

};