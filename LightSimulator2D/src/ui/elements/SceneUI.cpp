#include "SceneUI.h"

#include <imgui.h>

#include <string>

SceneUI* SceneUI::s_Instance = nullptr;

void SceneUI::Render()
{
	IM_BEGIN("Scene stats");

	ImGui::Text(("# Enttities: " + std::to_string(m_TotalEntities)).c_str());
	ImGui::Text(("# Instances: " + std::to_string(m_DrawnInstances) + " / " + std::to_string(m_TotalInstances)).c_str());
	ImGui::Spacing();
	ImGui::Text(("# Occluders: " + std::to_string(m_DrawnOccluders) + " / " + std::to_string(m_TotalOccluders)).c_str());
	ImGui::Text(("# Emitters: " + std::to_string(m_DrawnEmitters) + " / " + std::to_string(m_TotalEmitters)).c_str());
	IM_END();
}
