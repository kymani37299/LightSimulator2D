#pragma once

#include "config.h"
#include "core/Window.h"
#include "gfx/Renderer.h"
#include "input/Input.h"
#include "scene/Scene.h"

class PlayerController;

class GameEngine
{
private:
	static GameEngine* s_Instance;

public:
	static GameEngine* Get();
	static void Delete();

	Window* GetMainWindow() { return &m_Window; }
	Renderer* GetRenderer() { return &m_Renderer; }

public:
	void Run();
	void Stop() { m_Running = false; }

private:
	GameEngine() = default;
	~GameEngine();

	void Init();
	void EngineLoop();

	void UpdateDT();
	
	float m_DT=0.0f;
	bool m_Running = false;

	Window m_Window{ SCREEN_WIDTH, SCREEN_HEIGHT, PROJECT_NAME };
	Renderer m_Renderer;
	Input m_Input{ &m_Window };
	PlayerController* m_Controller;

	Scene m_Scene;
};