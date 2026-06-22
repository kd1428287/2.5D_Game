#pragma once
#include "../EffectBase.h"
#include <vector>

class Smoke : public EffectBase
{
public:
	Smoke(Math::Vector3 pos, float scale = 0.3f, bool point = false, float limit = 40.f);
	~Smoke() override {}

	void Init()             override;
	void Update(float dt)   override;
	void PostUpdate()       override;   // カメラ回転を取得
	void DrawEffect()       override;

private:
	struct Particle
	{
		Math::Vector3 pos;
		Math::Vector3 velocity;
		float         particleScale;
		float         lifetime;
		float         maxLifetime;
		float         swayPhase;
		float         swayFreq;
		float         swayAmplitude;
		bool          alive;
	};

	std::vector<Particle> m_particles;

	float m_emitTimer = 0.0f;
	float m_emitInterval = 0.08f;
	int   m_maxParticles = 32;

	int   m_emitCount = 0;   // これまでに生成した粒の総数
	int   m_emitLimit = 40;  // この数を超えたらエミッター停止

	// PostUpdate でキャッシュしたカメラ回転 → DrawEffect で使用
	Math::Quaternion m_cameraRot = Math::Quaternion::Identity;

	Particle SpawnParticle() const;
};