#include "CarDust.h"
#include <cmath>
#include <cstdlib>
#include "../../../System/Reader/Reader.h"

namespace
{
	float RandN11() { return (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f; }
	float Rand01() { return  static_cast<float>(rand()) / RAND_MAX; }
}

// ─────────────────────────────────────────────────────────────────
//  コンストラクタ
// ─────────────────────────────────────────────────────────────────
CarDust::CarDust(Math::Vector3 pos, float angleY,
	float wheelBase, float tread, float scale)
	: EffectBase(pos, scale, /*loop=*/false)
	, m_angleY(angleY)
	, m_wheelBase(wheelBase)
	, m_tread(tread)
{
	m_drawType = eDrawTypeUnLit;
	Init();
}

void CarDust::Init()
{
	m_particles.clear();
	m_particles.reserve(k_maxParticles);
	m_emitTimer = 0.0f;

	if (!m_polygon)
	{
		m_polygon = std::make_shared<KdSquarePolygon>(
			KdAssets::Instance().m_textures.GetData("Asset/Textures/Effect/Dot.png"));
	}
}

// ─────────────────────────────────────────────────────────────────
//  SetCarState  ─ 毎フレーム車体情報を同期
// ─────────────────────────────────────────────────────────────────
void CarDust::SetCarState(Math::Vector3 pos, float angleY, float speed)
{
	m_pos = pos;
	m_angleY = angleY;
	m_speed = speed;
}

// ─────────────────────────────────────────────────────────────────
//  後輪2本の座標を計算
//  車体中心から前後方向に -wheelBase/2、左右に ±tread/2
// ─────────────────────────────────────────────────────────────────
void CarDust::GetRearWheelPositions(Math::Vector3& outL, Math::Vector3& outR) const
{
	const float fwd_x = std::sin(m_angleY);
	const float fwd_z = std::cos(m_angleY);
	const float right_x = fwd_z;   // 右方向 = 前方を90°回転
	const float right_z = -fwd_x;

	// 後輪中心(前後方向に -wheelBase/2 ずらす)
	const float rx = m_pos.x - fwd_x * (m_wheelBase * 0.5f);
	const float rz = m_pos.z - fwd_z * (m_wheelBase * 0.5f);

	outL = { rx - right_x * (m_tread * 0.5f), m_pos.y, rz - right_z * (m_tread * 0.5f) };
	outR = { rx + right_x * (m_tread * 0.5f), m_pos.y, rz + right_z * (m_tread * 0.5f) };
}

// ─────────────────────────────────────────────────────────────────
//  SpawnParticle  ─ タイヤ1本分の粒を生成
//  ・後方に強く蹴り出し、左右に少し広がる
//  ・速度が高いほど粒が大きく長持ちする
// ─────────────────────────────────────────────────────────────────
CarDust::Particle CarDust::SpawnParticle(Math::Vector3 wheelPos) const
{
	Particle p;
	p.alive = true;

	// 発生座標: タイヤ接地点周囲に小さくばらす
	p.pos.x = wheelPos.x + RandN11() * 0.05f;
	p.pos.y = wheelPos.y;
	p.pos.z = wheelPos.z + RandN11() * 0.05f;

	// 速度に応じたスケール係数(低速でも最低限出る)
	const float speedFactor = std::max(m_speed / 10.0f, 0.2f);  // 10m/s基準

	// 初速: 後方を中心に扇状、速度に比例
	const float backAngle = m_angleY + 3.14159265f;
	const float spread = RandN11() * (3.14159265f / 4.0f);   // ±45°
	const float flyAngle = backAngle + spread;

	const float hSpeed = (1.5f + Rand01() * 1.0f) * speedFactor;
	p.velocity.x = std::sin(flyAngle) * hSpeed;
	p.velocity.z = std::cos(flyAngle) * hSpeed;
	p.velocity.y = (0.2f + Rand01() * 0.3f) * speedFactor;     // 速いほど高く舞う

	// 寿命: 速度が高いほど長持ち
	p.maxLifetime = (0.4f + Rand01() * 0.3f) * (0.5f + speedFactor * 0.5f);
	p.lifetime = p.maxLifetime;

	// スケール: 速度が高いほど大きい粒
	p.particleScale = (0.05f + Rand01() * 0.05f) * (0.5f + speedFactor);

	return p;
}

// ─────────────────────────────────────────────────────────────────
//  Update
// ─────────────────────────────────────────────────────────────────
void CarDust::Update(float dt)
{
	// ── エミッター ──────────────────────────────
	if (m_emitting && m_speed > 0.5f)   // 極低速では出さない
	{
		// 速いほど発生間隔を短く(最大4倍密)
		const float speedFactor = std::min(m_speed / 10.0f, 1.0f);
		const float interval = m_emitInterval / (1.0f + speedFactor * 3.0f);

		m_emitTimer += dt;
		while (m_emitTimer >= interval)
		{
			m_emitTimer -= interval;

			Math::Vector3 wheelL, wheelR;
			GetRearWheelPositions(wheelL, wheelR);

			// 左右後輪それぞれ1粒ずつ
			for (auto* wpos : { &wheelL, &wheelR })
			{
				bool spawned = false;
				for (auto& p : m_particles)
				{
					if (!p.alive) { p = SpawnParticle(*wpos); spawned = true; break; }
				}
				if (!spawned && static_cast<int>(m_particles.size()) < k_maxParticles)
					m_particles.push_back(SpawnParticle(*wpos));
			}
		}
	}

	// ── 各パーティクル更新 ──────────────────────
	bool anyAlive = false;
	for (auto& p : m_particles)
	{
		if (!p.alive) continue;

		p.lifetime -= dt;
		if (p.lifetime <= 0.0f) { p.alive = false; continue; }
		anyAlive = true;

		// 重力
		p.velocity.y -= 0.5f * dt;
		if (p.velocity.y < 0.0f) p.velocity.y = 0.0f;

		// 空気抵抗
		const float drag = 1.0f - 2.5f * dt;
		p.velocity.x *= drag;
		p.velocity.z *= drag;

		p.pos.x += p.velocity.x * dt;
		p.pos.y += p.velocity.y * dt;
		p.pos.z += p.velocity.z * dt;

		// スケール: sin カーブで膨張→収縮
		const float ratio = p.lifetime / p.maxLifetime;
		p.particleScale *= 1.0f;   // SpawnParticle で設定済みの値を基準に
		// DrawEffect 側で ratio を使ってサイズ変調するのでここは座標だけ動かす
	}

	// エミッター停止済み & 全粒消滅 → 自動消滅
	if (!m_emitting && !anyAlive)
		m_isExpired = true;
}

// ─────────────────────────────────────────────────────────────────
//  PostUpdate
// ─────────────────────────────────────────────────────────────────
void CarDust::PostUpdate()
{
	auto camData = Reader::Instance().ReadCamera();

	Math::Vector3    camScale, camTrans;
	Math::Quaternion camRot;
	camData.mat.Decompose(camScale, camRot, camTrans);

	m_cameraRot = camRot;
}

// ─────────────────────────────────────────────────────────────────
//  DrawEffect
// ─────────────────────────────────────────────────────────────────
void CarDust::DrawEffect()
{
	if (!m_polygon) return;

	for (const auto& p : m_particles)
	{
		if (!p.alive) continue;

		const float ratio = p.lifetime / p.maxLifetime;  // 1→0
		const float alpha = std::sin(ratio * 3.14159265f) * 0.65f;

		// 土埃の色: 速度が高いほど少し白っぽく(砂→土煙)
		const float w = std::min(m_speed / 15.0f, 0.3f);
		Math::Color color = { 0.78f + w, 0.65f + w, 0.45f + w, alpha };

		// スケールもサイン変調
		const float s = m_scale * p.particleScale * std::sin(ratio * 3.14159265f);
		Math::Matrix mat =
			Math::Matrix::CreateScale(s)
			* Math::Matrix::CreateFromQuaternion(m_cameraRot)
			* Math::Matrix::CreateTranslation(p.pos);

		KdShaderManager::Instance().m_StandardShader.DrawPolygon(
			*m_polygon, mat, color, Math::Vector3{ 0, 0, 0 });
	}
}