#include "FollowMouseComponent.h"

#include "core/Engine.h"
#include "input/Input.h"

#include "scene/Entity.h"

void FollowMouseComponent::Update(float dt)
{
	// Reverse camera transformation to go to world space
	const Mat3 V = GameEngine::Get()->GetCamera().GetTransformation();
	const Vec2 mousePos = GameEngine::Get()->GetInput()->GetMousePosition();
	const Vec3 position3D = Vec3(mousePos.x, mousePos.y, 1.0) * glm::inverse(V);
	const Vec2 targetPos = Vec2(position3D.x, position3D.y);

	for (EntityInstance* et : m_Parent->GetInstances())
	{
		et->SetPosition(targetPos);
	}
}