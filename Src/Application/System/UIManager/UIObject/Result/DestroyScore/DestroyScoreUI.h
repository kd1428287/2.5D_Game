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
	int m_score = 0;
	int m_digitCount = 0;

	// ドラムロール用
	float m_rollTimer = 0.f;
	float m_rollInterval = 0.05f;
	float m_digitDelay = 0.6f;

	static constexpr int MAX_DIGITS = 8;

	int   m_fixedFromRight = 0;
	int   m_currentRand[MAX_DIGITS] = {};
	float m_digitTimer = 0.f;

	bool  m_rolling = false;
	bool  m_finished = false;

	
	int   m_digits[MAX_DIGITS] = {};

	ScopedSubscriber m_rollSub;
};