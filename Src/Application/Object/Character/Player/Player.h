#pragma once

enum class SpeedLevel
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

class Player : public KdGameObject
{
public:
	Player() {}
	Player(Math::Vector3 pos) : m_pos(pos) {}
	Player(Math::Vector3 pos, Math::Vector3 angle) : m_pos(pos), m_angle(angle)
	{
		m_mWorld =
			Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_angle.y)) *
			Math::Matrix::CreateRotationX(DirectX::XMConvertToRadians(m_angle.x)) *
			Math::Matrix::CreateTranslation(pos);
	};
	~Player() override {}

	void Init()           override;
	void PreUpdate()      override;
	void Update(float dt) override;
	void PostUpdate()     override;

	void GenerateDepthMapFromLight() override;
	void DrawLit()                   override;

	void ChangeSpeedLevel(SpeedLevel level);

	SpeedLevel    GetSpeedLevel()    const { return m_level; }
	float         GetSpeed()         const { return m_speed; }
	float         GetSteeringInput() const { return m_steering / m_maxSteerAngle; }
	Math::Vector3 GetAngle()         const { return m_angle; }
	int           GetDeliveryScore() const { return m_deliveryScore; }

	// ドリフト中かどうか(エフェクト側から参照)
	bool          IsDrifting()       const { return m_isDrifting; }

	// 実際の移動方向(ドリフト中は車体向きとズレる) XZ正規化済み
	Math::Vector3 GetMoveDirection() const
	{
		Math::Vector3 dir = { m_velocity.x, 0.0f, m_velocity.z };
		float len = dir.Length();
		if (len > 0.001f) dir /= len;
		return dir;
	}

private:
	void UpdateMove(float dt);
	void UpdateGroundCollision(const std::vector<std::shared_ptr<KdGameObject>>& objList);
	void UpdateWallCollision(const std::vector<std::shared_ptr<KdGameObject>>& objList, const Math::Matrix& rotMat);
	void UpdateAutoPilotInput(float dt);

private:
	// --- モデル ---
	std::shared_ptr<KdModelData> m_model = nullptr;

	// --- 状態フラグ ---
	bool m_isControllable = false;
	bool m_isDeliveryAnime = false;

	// --- 配達アニメーション ---
	float m_deliveryAnimeTime = 0.0f;
	float m_deliveryAnimeAmplitude = 0.5f;
	float m_deliveryAnimeSpeed = 7.0f;

	// --- トランスフォーム ---
	Math::Vector3 m_pos;
	Math::Vector3 m_angle;
	Math::Vector3 m_scale;

	// --- 移動・速度 ---
	Math::Vector3 m_amountMove;
	Math::Vector3 m_moveVec;		// 車体の向きベクトル
	Math::Vector3 m_velocity;		// 【追加】実際の移動速度ベクトル(ドリフト用)
	float m_turnSpeed = 0.0f;
	float m_speed = 0.0f;
	float m_maxSpeed = 2.0f;
	float m_minSpeed = -2.0f;

	// --- 重力・落下 ---
	float       m_fallVelocity = 0.0f;
	float       m_fallDistance = 0.0f;
	const float GRAVITY_ACCEL = 9.8f;
	const float MAX_FALL_SPEED = -10.0f;

	// --- クラッシュ ---
	float m_clashCount = 0.0f;

	// --- 加速 ---
	float m_acceleration = 0.0f;

	// --- バイシクルモデル用ステアリング ---
	float m_steering = 0.0f;
	float m_steerSpeed = 90.0f;
	float m_maxSteerAngle = 35.0f;
	float m_wheelBase = 0.2f;

	// ================================================================
	// --- ドリフト ---
	// ================================================================
	bool  m_isDrifting = false;  // ドリフト状態フラグ
	bool  m_wasDrifting = false;  // 前フレームのドリフト状態(解除検出用)
	float m_driftDir = 0.0f;   // ドリフト開始時のステア方向(-1 or +1)
	float m_driftAngle = 0.0f;   // 速度ベクトルと車体向きのズレ角(ラジアン)
	float m_driftStartAngleRad = 0.0f;   // ドリフト開始時の車体向き(ラジアン)
	float m_lateralFriction = 8.0f;   // 通常時の横グリップ強度
	float m_driftFriction = 1.5f;   // ドリフト時の横グリップ強度(小さいほど滑る)

	// ドリフト中に開始角から曲げられる最大角度
	static constexpr float k_driftMaxRotationDeg = 65.0f;
	// ドリフト中のアクセル効率(1.0=通常と同じ、小さいほど控えめ)
	static constexpr float k_driftAccelScale = 0.6f;
	// ドリフト中の旋回レート倍率
	static constexpr float k_driftSteerScale = 1.8f;
	// ドリフト解除時の速度ブースト倍率
	static constexpr float k_driftReleaseBoost = 1.15f;
	// ドリフト開始に必要な最低速度
	static constexpr float k_driftTriggerSpeed = 1.1f;
	// ================================================================

	// --- 当たり判定 ---
	static constexpr int SPHERE_NUM = 3;
	const float         m_sphereOffsets[SPHERE_NUM] = { 0.07f, 0.0f, -0.07f };
	const float         m_sphereRadius = 0.05f;
	const Math::Vector3 m_sphereHeightOffset = { 0.0f, 0.06f, 0.0f };

	static constexpr int MAX_COLLISION_ITERATIONS = 3;
	std::set<std::shared_ptr<KdGameObject>> m_previousHitObjects;

	// --- スピードレベル ---
	SpeedLevel m_level = SpeedLevel::Idle;

	// --- スコア ---
	int m_deliveryScore = 0;
	int m_deliveryDestroy = 0;
	int m_destroyScore = 0;

	SpeedLevel m_autoPilotTargetLevel = SpeedLevel::Speed3;
};