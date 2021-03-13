#include "PlayerControllerComponent.h"

#include "scene/Entity.h"
#include "scene/Scene.h"

void PlayerControllerComponent::Update(float dt)
{
    dt /= 1000.0f;

    const Vec2 dir = m_MoveDir == VEC2_ZERO ? VEC2_ZERO : glm::normalize(m_MoveDir);
    const Vec2 walkVector = m_MoveDir * m_MovementSpeed * dt;
    if (walkVector != VEC2_ZERO)
        m_Parent->m_Transform.position += walkVector;

    const Vec2 cameraDir = m_CameraDir == VEC2_ZERO ? VEC2_ZERO : glm::normalize(m_CameraDir);
    const Vec2 cameraVector = m_CameraDir * m_CameraSpeed * dt;
    if (cameraVector != VEC2_ZERO)
        m_Camera.position += cameraVector;
}