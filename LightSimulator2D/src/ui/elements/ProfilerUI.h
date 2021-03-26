#pragma once

#include "ui/UIElement.h"

#include <vector>
#include <string>

class ProfilerDiagram : public UIElement
{
public:
	ProfilerDiagram(const std::string& label) : m_Label(label) {}

	void AddValue(float value);

	void Init() {}
	void Update(float dt) {}
	void Render();

	const std::string GetLabel() const { return m_Label; }

private:
	static constexpr unsigned MAX_VALUES = 100;

	std::string m_Label;
	std::vector<float> m_Values;
	float m_CurrentValue = -1.0;
};

class ProfilerUI : public UIElement
{
public:
	void Init() {}
	void Update(float dt);
	void Render();

private:
	void UpdateInternal();

private:
	static constexpr unsigned UPDATE_INTERVAL = 1000;

	float m_LastUpdateAge = 0;

	float m_CurrentFPS = 0.0f;
	std::vector<ProfilerDiagram*> m_Diagrams;
};

