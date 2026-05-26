#pragma once

class InputManager;
class CameraManager;
class TimeManager;

class GameEngine
{
public:

	static GameEngine& Instance()
	{
		static GameEngine instance;
		return instance;
	}

	void Init();
	void Update();
	
private:
	GameEngine() { Init(); };
	~GameEngine() {};
};