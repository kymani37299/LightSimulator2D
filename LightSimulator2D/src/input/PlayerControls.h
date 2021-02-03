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