#pragma once

#include "common.h"

#include <map>
#include <vector>

class Window;

enum class KeyState
{
	Invalid,
	Pressed,
	Released,
	Repeat
};

class KeyCallback
{
public:
	virtual void Callback() {}
	virtual void Callback(KeyState) {}

	void operator() () const = delete;
	void operator() () { Callback(); }

	void operator() (KeyState state) const = delete;
	void operator() (KeyState state) { Callback(state); }
};

#define KEY_CALLBACK_BEGIN(X) class X : public KeyCallback { void Callback() {
#define KEY_CALLBACK_END() }}

typedef unsigned int KeyInput;

class InputMap : public std::map<KeyInput, std::vector<KeyCallback*> >
{
public:
	inline bool Contains(KeyInput key) const { return find(key) != end(); }
};

class Input
{
	friend class Window;

public:
	Input(Window* window) : m_Window(window) {}

	void AddKeyInputCallback(KeyInput key, KeyCallback* callback);
	void AddKeyPressedCallback(KeyInput key, KeyCallback* callback);
	void AddStateChangedCallback(KeyInput key, KeyCallback* callback);

	Vec2 GetMousePosition();
private:
	void OnKeyInput(KeyInput key, KeyState state);

private:
	Window* m_Window;

	InputMap m_KeyInputCallbacks;
	InputMap m_KeyPressedCallbacks;
	InputMap m_StateChangedCallback;
};