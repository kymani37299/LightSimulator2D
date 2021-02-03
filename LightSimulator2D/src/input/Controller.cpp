#include "Controller.h"

#include "input/Input.h"

#include "input/PlayerControls.h"

#include "scene/Entity.h"

void PlayerController::Update(float dt)
{
    dt /= 1000.0f;

    const Vec2 dir = m_Movedir == VEC2_ZERO ? VEC2_ZERO : glm::normalize(m_Movedir);
    const Vec2 walkVector = m_Movedir * m_PlayerMovementSpeed * dt;
    if(walkVector!=VEC2_ZERO)
        m_PlayerEntity->m_Transform.position += walkVector;
}

void PlayerController::_Init()
{
    m_Input->AddStateChangedCallback(GLFW_KEY_W, PLAYER_KEY_CALLBACK(ForwardCallback));
    m_Input->AddStateChangedCallback(GLFW_KEY_S, PLAYER_KEY_CALLBACK(BackCallback));
    m_Input->AddStateChangedCallback(GLFW_KEY_A, PLAYER_KEY_CALLBACK(LeftCallback));
    m_Input->AddStateChangedCallback(GLFW_KEY_D, PLAYER_KEY_CALLBACK(RightCallback));

    m_Input->AddKeyPressedCallback(GLFW_KEY_ESCAPE, PLAYER_KEY_CALLBACK(EscapeCallback));
}