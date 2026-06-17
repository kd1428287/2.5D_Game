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

	bool m_isCounting = false;
	ScopedSubscriber m_startSub;
};