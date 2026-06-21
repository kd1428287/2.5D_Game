#pragma once
#include "../../UIObject.h"

class TitleTextUI : public UIObject
{
public:
	TitleTextUI() {};
	~TitleTextUI()override {};

	void Init()override;
	void Update(float dt)override;
	void DrawSprite()override;

private:
	bool m_isPressed = false;
	float m_time = 0.f;   // 経過時間（上下・明滅共用）

	// 上下ボブ
	float m_bobAmp = 10.f;  // 上下の振幅(px)
	float m_bobSpeed = 1.5f;  // 上下の周期(rad/s)

	// 明滅
	float m_blinkSpeed = 1.0f;  // 明滅の周期(rad/s)
	float m_fadeAlpha = 1.f; // 1→0 に減衰。明滅アルファに乗算する
	float m_alphaMin = 0.2f;  // 最小アルファ
	float m_alphaMax = 1.0f;  // 最大アルファ

	// 基準位置（画面中央下）
	Math::Vector3 m_basePos = { 0.f, -250.f,0.f };

	ScopedSubscriber m_startSub;
};