#pragma once

#include "common.h"

#include "scene/Component.h"

class PlayerControllerComponent : public Component
{
public:
	void Update(float dt) override;

	Vec2& GetMoveDir() { return m_MoveDir; }

private:

	float m_MovementSpeed = 1.0f;
	Vec2 m_MoveDir = VEC2_ZERO;
};