#include "Controller.h"

#include "input/Input.h"

#include "input/PlayerControls.h"

void PlayerController::Update(float dt)
{
}

void PlayerController::_Init()
{
    m_Input->AddKeyPressedCallback(GLFW_KEY_ESCAPE, PLAYER_KEY_CALLBACK(EscapeCallback));
}