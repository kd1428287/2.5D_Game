#pragma once

class InputManager;
class CameraManager;
class TimeManager;

class GameManager
{
public:

	static GameManager& Instance()
	{
		static GameManager instance;
		return instance;
	}

	void Init();
	void Update();
	
private:
	GameManager() { Init(); };
	~GameManager() {};
};