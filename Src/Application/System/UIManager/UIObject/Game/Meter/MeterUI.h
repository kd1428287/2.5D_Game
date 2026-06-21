#pragma once

#include "../../UIObject.h"

class MeterUI : public UIObject
{
public:
	MeterUI() {};
	~MeterUI()override {};

	void Init()override;
	void Update(float dt)override;
	void DrawSprite()override;

private:
	enum class NeedlePoint
	{
		Idle,
		Speed1,
		Speed2,
		Speed3,
		Speed4,
		Speed5,
		Speed6,
		Clash,
	};

	void SetTargetPoint(NeedlePoint point);

	// 画面サイズ定数 (1280x720)
	static constexpr float SCREEN_W = 1280.0f;
	static constexpr float SCREEN_H = 720.0f;

	// メーター本体テクスチャ
	std::shared_ptr<KdTexture> m_needle;	// arrow テクスチャ

	// arrow の現在位置・目標位置
	// SpeedLevel が高いほど左(X小)・上(Y大) に移動する
	Math::Vector2 m_arrowPos;			// 現在のarrow描画位置
	Math::Vector2 m_arrowTargetPos;		// 目標のarrow位置

	// arrow 移動の補間速度 (大きいほど速く追従)
	float m_arrowLerpSpeed = 5.0f;

	ScopedSubscriber m_speedSub;
};