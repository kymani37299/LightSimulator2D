#pragma once

#include "common.h"

enum class KeyState;
class Input;
class PlayerCamera;

class Entity;

class Controller
{
public:
	void Init(Input* input) { m_Input = input; _Init(); }
	virtual void Update(float dt) = 0;

private:
	virtual void _Init() = 0;

protected:
	Input* m_Input = nullptr;
};

class PlayerController : public Controller
{
public:
	PlayerController(Entity* playerEntity) : m_PlayerEntity(playerEntity) {}

	void Update(float dt);

	Vec2& GetMoveDir() { return m_Movedir; }

private:
	void _Init();

	Entity* m_PlayerEntity;
	float m_PlayerMovementSpeed = 1.0f;
	Vec2 m_Movedir = VEC2_ZERO;
};