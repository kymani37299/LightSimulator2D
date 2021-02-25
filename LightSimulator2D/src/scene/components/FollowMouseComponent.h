#pragma once

#include "scene/Component.h"

class FollowMouseComponent : public Component
{
public:
	void Update(float dt) override;
};