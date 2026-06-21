#pragma once
#include "../../UIObject.h"

class DeliveryScoreUI : public UIObject
{
public:
	DeliveryScoreUI() {};
	~DeliveryScoreUI()override {};

	void Init()override;
	void Update(float dt)override;
	void DrawSprite()override;

private:
	enum class RollPhase
	{
		Waiting,       // イベント待ち（全桁0表示）
		FirstRoll,     // 1回目ロール：m_score を右から確定
		WaitBetween,   // 1回目確定後の短い待機
		ShowTimeBonus, // "TimeBonus" を拡大→縮小表示
		WaitBetween2,  // TimeBonus消去後の短い待機
		SecondRoll,    // 2回目ロール：m_finalScore を全桁一斉→右から確定
		Done,
	};
	RollPhase m_phase = RollPhase::Waiting;

	int m_score = 0;
	int m_finalScore = 0;
	int m_digitCount = 0;

	float m_rollTimer = 0.f;
	float m_rollInterval = 0.05f;
	float m_digitDelay = 0.6f;
	float m_digitTimer = 0.f;
	int   m_fixedFromRight = 0;

	float m_phaseWait = 0.f;
	float m_phaseWaitMax = 0.4f; // WaitBetween / WaitBetween2 の待機時間(秒)

	// TimeBonus 演出
	float m_bonusTimer = 0.f;
	float m_bonusDuration = 1.8f; // 演出全体の長さ(秒)
	//  0.0〜0.4 : スケールアップ
	//  0.4〜1.4 : 静止表示
	//  1.4〜1.8 : フェードアウト
	float m_bonusScale = 0.f;
	float m_bonusAlpha = 0.f;

	static constexpr int MAX_DIGITS = 8;
	int m_digits[MAX_DIGITS] = {};
	int m_currentRand[MAX_DIGITS] = {};

	ScopedSubscriber m_rollSub;

	void SetupDigits(int value);
};