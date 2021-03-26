#include "ProfilerUI.h"

#include "util/Profiler.h"
#include "core/Engine.h"
#include "core/Window.h"

#include <imgui.h>
#include <algorithm>

static const std::string FToS(float v)
{
	return std::to_string(v).substr(0, 4);
}

static bool diagramComparator(const ProfilerDiagram* l, const ProfilerDiagram* r) 
{ 
	return l->GetLabel().compare(r->GetLabel()) < 0; 
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
	bool needSort = false;
	while (it != Profiler::s_CurrentState.end())
	{		if (std::find_if(m_Diagrams.begin(), m_Diagrams.end(), [&](ProfilerDiagram* pd) { return it->first == pd->GetLabel(); }) == m_Diagrams.end())
		{
			m_Diagrams.push_back(new ProfilerDiagram(it->first));
			needSort = true;
		}
		it++;
	}

	if (needSort) std::sort(m_Diagrams.begin(), m_Diagrams.end(), &diagramComparator);

	for (ProfilerDiagram* pd : m_Diagrams)
	{
		pd->AddValue(Profiler::GetTime(pd->GetLabel()));
	}

	m_CurrentFPS = GameEngine::Get()->GetMainWindow()->GetFps();
}

void ProfilerUI::Render()
{
	IM_BEGIN("Profiler");
	ImGui::Text(("FPS: " + FToS(m_CurrentFPS)).c_str());
	for (ProfilerDiagram* d : m_Diagrams)
	{
		d->Render();
	}
	IM_END();
}