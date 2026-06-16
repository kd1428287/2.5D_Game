#pragma once

class TimeManager
{
public:
	TimeManager() {};
	~TimeManager() {};

	void Init();
	void Update(float dt);
	void ChangeScene()

	float GetNowSceneTime() { return m_time; }

private:
	float m_time = 0.f;
};