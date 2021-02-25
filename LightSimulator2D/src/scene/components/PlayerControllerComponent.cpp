#include "PlayerControllerComponent.h"

#include "scene/Entity.h"

void PlayerControllerComponent::Update(float dt)
{
    dt /= 1000.0f;

    const Vec2 dir = m_MoveDir == VEC2_ZERO ? VEC2_ZERO : glm::normalize(m_MoveDir);
    const Vec2 walkVector = m_MoveDir * m_MovementSpeed * dt;
    if (walkVector != VEC2_ZERO)
        m_Parent->m_Transform.position += walkVector;
}