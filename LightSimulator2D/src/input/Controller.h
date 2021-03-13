#pragma once

#include "common.h"

enum class KeyState;
class Input;
class PlayerCamera;

class PlayerControllerComponent;

class Controller
{
public:
	void Init(Input* input) { m_Input = input; _Init(); }
	virtual void Update(float dt) {};

private:
	virtual void _Init() = 0;

protected:
	Input* m_Input = nullptr;
};

class PlayerController : public Controller
{
public:
	PlayerController(PlayerControllerComponent* controllerComponent) : m_ControllerComponent(controllerComponent) {}

	Vec2& GetMoveDir();
	Vec2& GetCameraDir();

private:
	void _Init();

	PlayerControllerComponent* m_ControllerComponent;
};