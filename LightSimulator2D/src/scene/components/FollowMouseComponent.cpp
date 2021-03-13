#include "FollowMouseComponent.h"

#include "core/Engine.h"
#include "input/Input.h"

#include "scene/Entity.h"

void FollowMouseComponent::Update(float dt)
{
	m_Parent->m_Transform.position = GameEngine::Get()->GetInput()->GetMousePosition(); // TODO: Sync this pos with camera pos
}