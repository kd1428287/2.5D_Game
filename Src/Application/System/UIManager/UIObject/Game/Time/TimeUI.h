#pragma once
#include "../../UIObject.h"

class TimeUI : public UIObject
{
public:
	TimeUI() {};
	~TimeUI()override {};

	void Init()override;
	void Update(float dt)override;
	void DrawSprite()override;
private:
	float m_time = 0.f;
};