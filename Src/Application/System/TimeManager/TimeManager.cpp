#include "TimeManager.h"
#include "../Reader/Reader.h"

void TimeManager::Init()
{
	m_time = 0.f;
	m_isCounting = false;
	m_startSub = GLOBALEVENT.subscribe<Events::Else::GameStart>
		([this](const Events::Else::GameStart& e)
			{
				m_isCounting = true;
			}
		);
}

void TimeManager::Update(float dt)
{
	if (!m_isCounting)return;
	m_time += dt;
	Reader::Instance().WriteTime(m_time);
	if ((int)m_time >= GAME_LIMIT)
	{
		GLOBALEVENT.publish(Events::Else::GameEnd());
		GLOBALEVENT.publish(Events::Else::GameToResultBegin());
		m_isCounting = false;
	}
}
