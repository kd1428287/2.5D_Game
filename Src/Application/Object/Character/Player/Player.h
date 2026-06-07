#pragma once

enum class SpeedLevel
{
	Idle,
	Speed1,
	Speed2,
	Speed3,
	Speed4,
	Speed5,
	Clash,
};

class Player : public KdGameObject
{
public:
	Player() {};
	Player(Math::Vector3 pos) : m_pos(pos) {};
	~Player()override {};

	void Init()override;
	void PreUpdate()override;
	void Update(float dt)override;
	void PostUpdate()override;
	
	void GenerateDepthMapFromLight()override;
	void DrawLit()override;

	void ChangeSpeedLevel(SpeedLevel level);
	SpeedLevel GetSpeedLevel() { return m_level; }

	float GetSteeringInput() { return m_steering / m_maxSteerAngle; }
	Math::Vector3 GetAngle() { return m_angle; }

private:
	void ActiveInput();
	void UpdateMove(float dt);

private:
	std::shared_ptr<KdModelData> m_model = nullptr;

	Math::Vector3 m_pos;
	Math::Vector3 m_angle;

	Math::Vector3 m_amountMove;
	Math::Vector3 m_moveVec;
	float m_speed = 0.0f;				// 現在のスピード
	float m_maxSpeed = 2.f;				// 最大のスピード
	float m_minSpeed = -2.f;			// 最低のスピード
	float m_gravity = 0.0f;				// 重力

	float m_clashCount = 0.0f;

	float m_acceleration = 0.0f;		// 加速力

	// --- 車の挙動（バイシクルモデル）用パラメータ ---
	float m_steering = 0.0f;			// 現在のステアリング角（前輪の角度：度数法）
	float m_steerSpeed = 90.0f;			// ハンドルを切る速さ（度/秒）
	float m_maxSteerAngle = 35.0f;		// ハンドルの最大切れ角（度）
	float m_wheelBase = 2.0f;			// ホイールベース（前輪から後輪までの距離）

	SpeedLevel m_level = SpeedLevel::Idle;
};