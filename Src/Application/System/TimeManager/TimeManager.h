#pragma once

class TimeManager
{
public:
	TimeManager() {};
	~TimeManager() {};

	void Init();
	void Update(float dt);
private:
	float m_time = 0.f;
	static const int GAME_LIMIT = 90;

	bool m_isCounting = false;
	ScopedSubscriber m_startSub;
	ScopedSubscriber m_endSub;
};