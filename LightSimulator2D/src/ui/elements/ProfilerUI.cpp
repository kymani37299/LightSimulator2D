#include "ProfilerUI.h"

#include "util/Profiler.h"
#include "core/Engine.h"
#include "core/Window.h"

#include <imgui.h>

static const std::string FToS(float v)
{
	return std::to_string(v).substr(0, 4);
}

void ProfilerDiagram::AddValue(float value)
{
	if (m_Values.size() >= MAX_VALUES)
	{
		m_Values.clear();
	}
	m_Values.push_back(value);
	m_CurrentValue = value;
}

void ProfilerDiagram::Render()
{
	ImGui::Text((m_Label + ":" + FToS(m_CurrentValue) + " (ms)").c_str());
	ImGui::PlotLines("", m_Values.data(), m_Values.size());
}

void ProfilerUI::Update(float dt)
{
	m_LastUpdateAge += dt;
	if (m_LastUpdateAge > UPDATE_INTERVAL)
	{
		UpdateInternal();
		m_LastUpdateAge = 0;
	}
}

void ProfilerUI::UpdateInternal()
{
	auto it = Profiler::s_CurrentState.begin();
	while (it != Profiler::s_CurrentState.end())
	{
		const std::string& profile = it->first;
		if (m_DiagramMap.find(profile) == m_DiagramMap.end())
		{
			m_DiagramMap[profile] = new ProfilerDiagram(profile);
		}
		m_DiagramMap[profile]->AddValue(Profiler::GetTime(profile));
		it++;
	}

	m_CurrentFPS = GameEngine::Get()->GetMainWindow()->GetFps();
}

void ProfilerUI::Render()
{
	IM_BEGIN("Profiler");
	ImGui::Text(("FPS: " + FToS(m_CurrentFPS)).c_str());
	for (auto it = m_DiagramMap.begin(); it != m_DiagramMap.end(); it++)
	{
		(*it).second->Render();
	}
	IM_END();
}