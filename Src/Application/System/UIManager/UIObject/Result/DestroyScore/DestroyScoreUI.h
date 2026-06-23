#pragma once
#include "../../UIObject.h"
#include "../../../../../Object/Number/Number.h"

class DestroyScoreUI : public UIObject
{
public:
	DestroyScoreUI() {};
	// center : 数字群全体の中心座標（設定する基準点）
	DestroyScoreUI(Math::Vector3 center, float dir = 180.f)
		: m_center(center), m_dir(dir) {};
	~DestroyScoreUI()override {};

	void Init()override;
	void Update(float dt)override;
	void GenerateDepthMapFromLight()override;
	void DrawLit()override;

private:
	// 桁ごとの Number を生成する
	void SetScore(int score);

	// 桁ごとの Number の行列を現在の n->m_pos から更新する
	void UpdateMatrix();

	// m_center と桁数から各桁のX座標を計算する
	// digitIndex : 0=一の位, 1=十の位, ...（右から左へインデックスが増える）
	// 例) 3桁スコアなら中心を0として 百の位=-SPACING, 十の位=0, 一の位=+SPACING
	float CalcDigitX(int digitIndex) const;

	static constexpr int   MAX_DIGITS = 4;		// 対応する最大桁数
	static constexpr float DIGIT_SPACING = 0.4f;	// 桁間隔(ワールド単位)

	Math::Vector3 m_center;			// 数字群全体の中心座標（外から設定する基準点）
	float         m_dir = 180.f;	// 向き[deg]

	int m_score = 0;
	int m_digitCount = 0;			// 現在表示中の桁数

	// 各桁の Number（[0]=一の位, [1]=十の位, ... ）
	std::array<std::shared_ptr<Number>, MAX_DIGITS> m_digits;

	// ---- 落下演出 ----
	bool          m_isFalling = false;
	Math::Vector3 m_velocity = {};		// 現在の速度ベクトル

	static constexpr float GRAVITY = -9.8f;	// 重力加速度[m/s²]
	static constexpr float INITIAL_VEL_Y = 2.0f;	// 落下開始時の初速(上方向)[m/s]

	ScopedSubscriber m_resPrbSub;
};