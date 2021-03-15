#include "FollowMouseComponent.h"

#include "core/Engine.h"
#include "input/Input.h"

#include "scene/Entity.h"

void FollowMouseComponent::Update(float dt)
{
	// Reverse camera transformation to go to world space
	Mat3 V = GameEngine::Get()->GetCamera().GetTransformation();
	Vec2 mousePos = GameEngine::Get()->GetInput()->GetMousePosition();
	Vec3 position3D = Vec3(mousePos.x, mousePos.y, 1.0) * glm::inverse(V);
	m_Parent->m_Transform.position = Vec2(position3D.x, position3D.y);
}