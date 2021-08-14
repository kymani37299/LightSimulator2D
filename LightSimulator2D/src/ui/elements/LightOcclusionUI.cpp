#include "LightOcclusionUI.h"

#include "core/Engine.h"
#include "gfx/Renderer.h"
#include "gfx/LightOcclusionRenderer.h"

#include <imgui.h>

void LightOcclusionUI::Update(float dt)
{
	unsigned int debugOptions = 0;
	if (m_DisableAngledRays) debugOptions |= OcclusionDebug_DisableAngledRays;
	if (m_DebugIntersections) debugOptions |= OcclusionDebug_Intersections;
	if (m_DebugRays) debugOptions |= OcclusionDebug_Rays;
	if (m_DebugMeshes) debugOptions |= OcclusionDebug_Mesh;
	if (m_SimpleLightMask) debugOptions |= OcclusionDebug_SimpleLightMask;
	GameEngine::Get()->GetRenderer()->GetLightOcclusionRenderer()->SetDebugOptions(debugOptions);
}

void LightOcclusionUI::Render()
{
	IM_BEGIN("Light Occlusion Debug");
	ImGui::Checkbox("Disable angled rays:", &m_DisableAngledRays);
	ImGui::Checkbox("Show intersections:", &m_DebugIntersections);
	ImGui::Checkbox("Show rays:", &m_DebugRays);
	ImGui::Checkbox("Show occlusion meshes:", &m_DebugMeshes);
	ImGui::Checkbox("Simple light mask:", &m_SimpleLightMask);
	IM_END();
}