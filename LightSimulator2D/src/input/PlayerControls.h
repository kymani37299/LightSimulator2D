#pragma once

#include "input/Input.h"
#include "input/Controller.h"

#include "core/Engine.h"

#define PLAYER_KEY_CALLBACK(X) new X(*this)
#define PLAYER_KEY_SCALLBACK_BEGIN(X) class X : public KeyCallback { public: X(PlayerController& _c):c(_c){} PlayerController& c;void Callback(KeyState state) {
#define PLAYER_KEY_CALLBACK_BEGIN(X) class X : public KeyCallback { public: X(PlayerController& _c):c(_c){} PlayerController& c;void Callback() {
#define PLAYER_KEY_CALLBACK_END() }};

PLAYER_KEY_CALLBACK_BEGIN(EscapeCallback)
    GameEngine::Get()->Stop();
PLAYER_KEY_CALLBACK_END()

PLAYER_KEY_SCALLBACK_BEGIN(ForwardCallback)
const float factor = state == KeyState::Pressed ? 1.0f : -1.0f;
c.GetMoveDir() += factor * Vec2(0.0f, 1.0f);
PLAYER_KEY_CALLBACK_END()

PLAYER_KEY_SCALLBACK_BEGIN(BackCallback)
const float factor = state == KeyState::Pressed ? 1.0f : -1.0f;
c.GetMoveDir() -= factor * Vec2(0.0f, 1.0f);
PLAYER_KEY_CALLBACK_END()

PLAYER_KEY_SCALLBACK_BEGIN(LeftCallback)
const float factor = state == KeyState::Pressed ? 1.0f : -1.0f;
c.GetMoveDir() -= factor * Vec2(1.0f, 0.0f);
PLAYER_KEY_CALLBACK_END()

PLAYER_KEY_SCALLBACK_BEGIN(RightCallback)
const float factor = state == KeyState::Pressed ? 1.0f : -1.0f;
c.GetMoveDir() += factor * Vec2(1.0f, 0.0f);
PLAYER_KEY_CALLBACK_END()

PLAYER_KEY_CALLBACK_BEGIN(ReloadShadersCallback)
GameEngine::Get()->GetRenderer()->ReloadShaders();
PLAYER_KEY_CALLBACK_END()

PLAYER_KEY_SCALLBACK_BEGIN(CamForwardCallback)
const float factor = state == KeyState::Pressed ? 1.0f : -1.0f;
c.GetCameraDir() += factor * Vec2(0.0f, 1.0f);
PLAYER_KEY_CALLBACK_END()

PLAYER_KEY_SCALLBACK_BEGIN(CamBackCallback)
const float factor = state == KeyState::Pressed ? 1.0f : -1.0f;
c.GetCameraDir() -= factor * Vec2(0.0f, 1.0f);
PLAYER_KEY_CALLBACK_END()

PLAYER_KEY_SCALLBACK_BEGIN(CamLeftCallback)
const float factor = state == KeyState::Pressed ? 1.0f : -1.0f;
c.GetCameraDir() -= factor * Vec2(1.0f, 0.0f);
PLAYER_KEY_CALLBACK_END()

PLAYER_KEY_SCALLBACK_BEGIN(CamRightCallback)
const float factor = state == KeyState::Pressed ? 1.0f : -1.0f;
c.GetCameraDir() += factor * Vec2(1.0f, 0.0f);
PLAYER_KEY_CALLBACK_END()
