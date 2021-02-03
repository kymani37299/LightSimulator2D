#pragma once

#include "common.h"

enum class KeyState;
class Input;
class PlayerCamera;

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
	PlayerController() {}
	
	void Update(float dt);

private:
	void _Init();
};