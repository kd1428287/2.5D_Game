#include "Smoke.h"
#include <cmath>
#include <cstdlib>
#include "../../../System/Reader/Reader.h"

namespace
{
	float RandN11() { return (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f; }
	float Rand01() { return  static_cast<float>(rand()) / RAND_MAX; }
}

Smoke::Smoke(Math::Vector3 pos, float scale, bool loop, float limit)
	: EffectBase(pos, scale, loop), m_emitLimit(limit)
{
	m_drawType = eDrawTypeUnLit;
	Init();
}

void Smoke::Init()
{
	m_particles.clear();
	m_particles.reserve(m_maxParticles);
	m_emitTimer = 0.0f;
	m_emitCount = 0;

	if (!m_polygon)
	{
		m_polygon = std::make_shared<KdSquarePolygon>(
			KdAssets::Instance().m_textures.GetData("Asset/Textures/Effect/Smoke.png"));
	}
}

Smoke::Particle Smoke::SpawnParticle() const
{
	Particle p;
	p.alive = true;

	// 発生位置: m_pos 中心に小さくばらす
	if (m_loopFlg)
	{
		p.pos = m_pos;
	}
	else
	{
		p.pos.x = m_pos.x + RandN11() * 0.1f;
		p.pos.y = m_pos.y;
		p.pos.z = m_pos.z + RandN11() * 0.1f;
	}

	// 初速: 主に上方向
	p.velocity.x = RandN11() * 0.04f;
	p.velocity.y = 0.5f + Rand01() * 0.3f;  // 0.5 〜 0.8 (m/s)
	p.velocity.z = RandN11() * 0.04f;

	// 寿命
	p.maxLifetime = 1.0f + Rand01() * 0.8f; // 1.0 〜 1.8 秒
	p.lifetime = p.maxLifetime;

	// 個別スケール: 初期は小さく → Update で膨らませる
	p.particleScale = 0.05f;

	// ふよふよ
	p.swayPhase = Rand01() * 6.2831f;
	p.swayFreq = 1.0f + Rand01() * 1.5f;
	p.swayAmplitude = 0.2f + Rand01() * 0.3f;

	return p;
}

void Smoke::Update(float dt)
{
	// ── エミッター ──────────────────────────
	if (m_emitCount < m_emitLimit)
	{
		m_emitTimer += dt;
		while (m_emitTimer >= m_emitInterval && m_emitCount < m_emitLimit)
		{
			m_emitTimer -= m_emitInterval;

			bool spawned = false;
			for (auto& p : m_particles)
			{
				if (!p.alive) { p = SpawnParticle(); spawned = true; break; }
			}
			if (!spawned && static_cast<int>(m_particles.size()) < m_maxParticles)
				m_particles.push_back(SpawnParticle());

			++m_emitCount;
		}
	}

	// ── 各パーティクル更新 ─────────────────
	bool anyAlive = false;
	for (auto& p : m_particles)
	{
		if (!p.alive) continue;

		p.lifetime -= dt;
		if (p.lifetime <= 0.0f) { p.alive = false; continue; }
		anyAlive = true;

		const float ratio = p.lifetime / p.maxLifetime; // 1→0
		const float elapsed = p.maxLifetime - p.lifetime;

		// 上昇
		p.velocity.y -= 0.15f * dt;
		p.velocity.y = std::max(p.velocity.y, 0.03f);

		// ふよふよ
		const float swayX = p.swayAmplitude
			* std::sin(p.swayFreq * elapsed * 6.2831f + p.swayPhase);
		const float swayZ = p.swayAmplitude * 0.7f
			* std::sin(p.swayFreq * elapsed * 6.2831f + p.swayPhase + 1.5707f);

		p.pos.x += (p.velocity.x + swayX) * dt;
		p.pos.y += p.velocity.y * dt;
		p.pos.z += (p.velocity.z + swayZ) * dt;

		// スケール:
		//   前半(ratio 1→0.5): 0.05 → 1.0 に膨張
		//   後半(ratio 0.5→0): 1.0 → 0.0 に縮小
		if (ratio > 0.5f)
			p.particleScale = 1.0f - (ratio - 0.5f) / 0.5f * (1.0f - 0.05f);
		// ratio=1 → 0.05, ratio=0.5 → 1.0
		else
			p.particleScale = ratio / 0.5f; // ratio=0.5 → 1.0, ratio=0 → 0.0
	}

	// 上限まで生成済み & 全粒が死んだら自分を消す
	if (m_emitCount >= m_emitLimit && !anyAlive)
		m_isExpired = true;
}

void Smoke::PostUpdate()
{
	// Exprosion と同じ方法でカメラの回転だけ取り出す
	auto camData = Reader::Instance().ReadCamera();

	Math::Vector3    camScale, camTrans;
	Math::Quaternion camRot;
	camData.mat.Decompose(camScale, camRot, camTrans);

	// 各パーティクルのビルボード行列をキャッシュ
	// (DrawEffect で使うため PostUpdate で確定させる)
	m_cameraRot = camRot;
}

void Smoke::DrawEffect()
{
	if (!m_polygon) return;

	for (const auto& p : m_particles)
	{
		if (!p.alive) continue;

		const float alpha = p.lifetime / p.maxLifetime;

		// Exprosion と同じ合成: Scale * CameraRot * Translation
		const float s = m_scale * p.particleScale;
		Math::Matrix mat =
			Math::Matrix::CreateScale(s)
			* Math::Matrix::CreateFromQuaternion(m_cameraRot)
			* Math::Matrix::CreateTranslation(p.pos);

		Math::Color color = { 0.85f, 0.85f, 0.85f, alpha };
		KdShaderManager::Instance().m_StandardShader.DrawPolygon(
			*m_polygon, mat, color, Math::Vector3{ 0, 0, 0 });
	}
}