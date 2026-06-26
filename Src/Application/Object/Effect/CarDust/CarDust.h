#pragma once
#include "../EffectBase.h"
#include <vector>

// ─────────────────────────────────────────────
//  CarDust  ─ 車走行時の4輪タイヤ土埃エフェクト
//
//  使い方:
//    // 走行開始時に生成
//    m_dustEffect = std::make_shared<CarDust>(carPos, angleY, wheelBase, tread);
//    scene->AddObject(m_dustEffect);
//
//    // 毎フレーム車体情報を更新
//    m_dustEffect->SetCarState(carPos, angleY, speed);
//
//    // 走行終了時
//    m_dustEffect->Stop();
//
//  引数:
//    pos       : 車体中心座標
//    angleY    : 車の向き(ラジアン, Y軸回転)
//    wheelBase : 前後輪間距離(デフォルト1.3f)
//    tread     : 左右輪間距離(デフォルト0.8f)
// ─────────────────────────────────────────────

class CarDust : public EffectBase
{
public:
	CarDust(Math::Vector3 pos, float angleY,
		float wheelBase = 1.3f, float tread = 0.8f,
		float scale = 0.25f);
	~CarDust() override {}

	void Init()           override;
	void Update(float dt) override;
	void PostUpdate()     override;
	void DrawEffect()     override;

	// 毎フレーム呼んで車体状態を同期
	//   pos    : 車体中心座標
	//   angleY : 車の向き(ラジアン)
	//   speed  : 速度(m/s) ── 速いほど粒が多く大きくなる
	void SetCarState(Math::Vector3 pos, float angleY, float speed);

	// 走行終了 ── エミッターを止めて残存粒が消えたら自動消滅
	void Stop() { m_emitting = false; }

private:
	struct Particle
	{
		Math::Vector3 pos;
		Math::Vector3 velocity;
		float         particleScale;
		float         lifetime;
		float         maxLifetime;
		bool          alive;
	};

	std::vector<Particle> m_particles;

	// 車体パラメータ
	float m_angleY = 0.0f;
	float m_wheelBase = 1.3f;   // 前後輪間距離
	float m_tread = 0.8f;   // 左右輪間距離
	float m_speed = 0.0f;   // 現在速度(m/s)

	// エミッター
	bool  m_emitting = true;
	float m_emitTimer = 0.0f;
	float m_emitInterval = 0.05f;   // 1輪あたりの発生間隔(秒)

	static constexpr int k_maxParticles = 128;

	Math::Quaternion m_cameraRot = Math::Quaternion::Identity;

	// タイヤ座標を計算して返す(後輪2本)
	void GetRearWheelPositions(Math::Vector3& outL, Math::Vector3& outR) const;

	Particle SpawnParticle(Math::Vector3 wheelPos) const;
};