#pragma once
#include "../../UIObject.h"
#include "../../../../../Object/Number/Number.h"

class DeliveryScoreUI : public UIObject
{
public:
	DeliveryScoreUI() {};
	~DeliveryScoreUI()override {};

	void Init()override;
	void Update(float dt)override;
	void GenerateDepthMapFromLight()override;
	void DrawLit()override;

private:
	// m_pos / m_dir / アニメーション変数から m_number の m_mWorld を再構築する
	void RebuildMatrix();

	// 1桁ずつ個別に保持する
	std::shared_ptr<Number> m_numberTens;	// 十の位
	std::shared_ptr<Number> m_numberOnes;	// 一の位

	int   m_score = 0;
	Math::Vector3 m_pos = { 1.5f,0.5f,1.5f };		// ベース位置
	float m_dir = 160.f;		// ベース向き[deg]

	// ---- アニメーション変数 ----
	Math::Vector3 m_animOffset = {};   // 位置オフセット
	float         m_animScale = 1.f;  // スケール倍率
	float         m_animExtraDir = 0.f;  // 追加回転角度[deg]

	ScopedSubscriber m_resPrbSub;

	// ---- 演出1 : Delivery (カクっと下げて数字を変え、戻ってくる) ----
	enum class DropState { Idle, DroppingDown, WaitBottom, RisingUp };
	DropState m_dropState = DropState::Idle;
	float     m_dropTimer = 0.f;
	int       m_pendingScore = 0;	// 最下点で適用するスコア

	static constexpr float DROP_DURATION = 0.12f;	// 下がりきるまでの時間[s]
	static constexpr float DROP_HOLD = 0.04f;	// 最下点で静止する時間[s]
	static constexpr float RISE_DURATION = 0.12f;	// 元の位置に戻るまでの時間[s]
	static constexpr float DROP_AMOUNT = 0.3f;	// 下げる量(ワールド単位)

	// ---- 演出2 : Completed (くるっと回って一瞬大きくなる) ----
	enum class SpinState { Idle, Spinning };
	SpinState m_spinState = SpinState::Idle;
	float     m_spinTimer = 0.f;

	static constexpr float SPIN_DURATION = 0.4f;		// 1回転にかける時間[s]
	static constexpr float SPIN_PEAK_SCALE = 1.5f;		// ピーク時のスケール倍率

	// ---- 2桁化用の定数 ----
	static constexpr float GAP_WIDTH = 0.3f;	// 数字と数字の間隔（モデルのサイズに合わせて調整してください）
};