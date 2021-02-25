#include "Controller.h"

#include "input/Input.h"

#include "input/PlayerControls.h"

#include "scene/components/PlayerControllerComponent.h"

void PlayerController::_Init()
{
    m_Input->AddStateChangedCallback(GLFW_KEY_W, PLAYER_KEY_CALLBACK(ForwardCallback));
    m_Input->AddStateChangedCallback(GLFW_KEY_S, PLAYER_KEY_CALLBACK(BackCallback));
    m_Input->AddStateChangedCallback(GLFW_KEY_A, PLAYER_KEY_CALLBACK(LeftCallback));
    m_Input->AddStateChangedCallback(GLFW_KEY_D, PLAYER_KEY_CALLBACK(RightCallback));

    m_Input->AddKeyPressedCallback(GLFW_KEY_ESCAPE, PLAYER_KEY_CALLBACK(EscapeCallback));
    m_Input->AddKeyPressedCallback(GLFW_KEY_R, PLAYER_KEY_CALLBACK(ReloadShadersCallback));
}

Vec2& PlayerController::GetMoveDir()
{
    return m_ControllerComponent->GetMoveDir();
}