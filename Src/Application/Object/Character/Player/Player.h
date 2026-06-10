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
	float GetSpeed() { return m_speed; }

	float GetSteeringInput() { return m_steering / m_maxSteerAngle; }
	Math::Vector3 GetAngle() { return m_angle; }

private:
	void UpdateMove(float dt);
	void UpdateGroundCollision(const std::vector<std::shared_ptr<KdGameObject>>& objList);
	void UpdateWallCollision(const std::vector<std::shared_ptr<KdGameObject>>& objList, const Math::Matrix& rotMat);

private:
	std::shared_ptr<KdModelData> m_model = nullptr;

	Math::Vector3 m_pos;				// 確定座標
	Math::Vector3 m_angle;				// 車体方向

	Math::Vector3 m_amountMove;			// (1Fでの)移動距離
	Math::Vector3 m_moveVec;			// 移動方向
	float m_speed = 0.0f;				// 現在のスピード
	float m_maxSpeed = 2.f;				// 最大のスピード
	float m_minSpeed = -2.f;			// 最低のスピード

	float m_fallVelocity = 0.0f;           // 現在の垂直方向の速度（落下速度）
	float m_fallDistance = 0.0f;		   // 現在の落下距離
	const float GRAVITY_ACCEL = 9.8f;      // 重力加速度 (定数)
	const float MAX_FALL_SPEED = -10.0f;   // 終端速度（落下速度の上限値）

	float m_clashCount = 0.0f;

	float m_acceleration = 0.0f;		// 加速力

	// --- 車の挙動（バイシクルモデル）用パラメータ ---
	float m_steering = 0.0f;			// 現在のステアリング角（前輪の角度：度数法）
	float m_steerSpeed = 90.0f;			// ハンドルを切る速さ（度/秒）
	float m_maxSteerAngle = 35.0f;		// ハンドルの最大切れ角（度）
	float m_wheelBase = 0.2f;			// ホイールベース（前輪から後輪までの距離）

	// --- 車の当たり判定用パラメータ ---
	static constexpr int   SPHERE_NUM = 3;
	const float            m_sphereOffsets[SPHERE_NUM] = { 0.07f, 0.0f, -0.07f }; // 前・中・後
	const float            m_sphereRadius = 0.05f;     // 球の半径
	const Math::Vector3    m_sphereHeightOffset = { 0.0f, 0.06f, 0.0f }; // 高さ補正

	// 物理計算の安全弁
	static constexpr int   MAX_COLLISION_ITERATIONS = 3; // コーナーハメを防ぐ最大反復回数
	std::set<std::shared_ptr<KdGameObject>> m_previousHitObjects;

	SpeedLevel m_level = SpeedLevel::Idle;
};