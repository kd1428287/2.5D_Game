#pragma once
#include "../../UIObject.h"

class DestroyScoreUI : public UIObject
{
public:
	DestroyScoreUI() {};
	~DestroyScoreUI()override {};

	void Init()override;
	void Update(float dt)override;
	void DrawSprite()override;

private:
	Math::Vector3 m_scorePos;
	int m_score = 0;
	int m_digitCount = 0;

	ScopedSubscriber m_resPrbSub;
};