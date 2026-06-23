#pragma once
#include "../Player/Player.h"

//enum class SpeedLevel
//{
//	Idle,
//	Speed1,
//	Speed2,
//	Speed3,
//	Speed4,
//	Speed5,
//	Speed6,
//	Clash,
//};

class AutoPlayer : public Player
{
public:
	AutoPlayer() {}
	AutoPlayer(Math::Vector3 pos) : m_pos(pos) {}
	AutoPlayer(Math::Vector3 pos, Math::Vector3 angle) : m_pos(pos), m_angle(angle)
	{
		m_scale = { 1.0f, 1.0f, 1.0f };
		m_mWorld =
			Math::Matrix::CreateScale(m_scale) *
			Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_angle.y)) *
			Math::Matrix::CreateRotationX(DirectX::XMConvertToRadians(m_angle.x)) *
			Math::Matrix::CreateTranslation(pos);
	};
	~AutoPlayer() override;

	void Init()         override;
	void PreUpdate()    override;
	void Update(float dt) override;
	void PostUpdate()   override;

	void GenerateDepthMapFromLight() override;
	void DrawLit()                   override;

	void ChangeSpeedLevel(SpeedLevel level);

	SpeedLevel    GetSpeedLevel()    const { return m_level; }
	float         GetSpeed()         const { return m_speed; }
	Math::Vector3 GetAngle()         const { return m_angle; }
	bool          IsActive()         const { return m_isActive; }

	// 自動操縦 API
	// waypoints を渡すと順番に巡回する。空の場合は直進し続ける。
	void StartAutoPilot(const std::vector<Math::Vector3>& waypoints = {});
	void StopAutoPilot();   // 停止後は速度が自然減衰して止まる

private:
	void UpdateMove(float dt);
	void UpdateGroundCollision(const std::vector<std::shared_ptr<KdGameObject>>& objList);
	void UpdateWallCollision(const std::vector<std::shared_ptr<KdGameObject>>& objList, const Math::Matrix& rotMat);
	void UpdateAutoPilotInput(float dt);    // 自動操縦の仮想入力を毎フレーム計算する
	void UpdateAction(float dt);

private:
	// --- モデル ---
	std::shared_ptr<KdModelData> m_model = nullptr;

	// --- スコア ---
	int m_deliveryScore = 0;
	int m_deliveryDestroy = 0;
	int m_destroyScore = 0;

	// --- 状態フラグ ---
	bool m_isActive = false;         // 自動操縦が稼働中かどうか
	bool m_isDeliveryAnime = false;

	// --- 配達アニメーション ---
	float m_deliveryAnimeTime = 0.0f;        // 経過タイマー
	float m_deliveryAnimeAmplitude = 0.5f;   // 振れ幅
	float m_deliveryAnimeSpeed = 7.0f;       // 速度

	bool m_isAction = false;
	float m_actionWait = 0.5f;

	// --- トランスフォーム ---
	Math::Vector3 m_pos;    // 確定座標
	Math::Vector3 m_angle;  // 車体方向
	Math::Vector3 m_scale = { 1.0f, 1.0f, 1.0f };  // 車体の大きさ

	// --- 移動・速度 ---
	Math::Vector3 m_amountMove;          // 1F での移動距離
	float m_speed = 0.0f;                // 現在のスピード
	float m_maxSpeed = 2.0f;             // 最大のスピード
	float m_minSpeed = -2.0f;            // 最低のスピード

	// --- 重力・落下 ---
	float       m_fallVelocity = 0.0f;    // 現在の垂直方向の速度
	float       m_fallDistance = 0.0f;    // 現在の落下距離
	const float GRAVITY_ACCEL = 9.8f;     // 重力加速度
	const float MAX_FALL_SPEED = -10.0f;  // 終端速度

	// --- クラッシュ ---
	float m_clashCount = 0.0f;   // クラッシュ中の操作不能残り時間

	// --- 加速 ---
	float m_acceleration = 0.0f;

	// --- バイシクルモデル用ステアリング ---
	float m_steering = 0.0f;        // 現在のステアリング角（度）
	float m_wheelBase = 0.2f;       // ホイールベース（前輪〜後輪の距離）

	// --- 当たり判定 ---
	static constexpr int SPHERE_NUM = 3;
	const float         m_sphereOffsets[SPHERE_NUM] = { 0.07f, 0.0f, -0.07f };  // 前・中・後
	const float         m_sphereRadius = 0.05f;
	const Math::Vector3 m_sphereHeightOffset = { 0.0f, 0.06f, 0.0f };

	// コーナーハメを防ぐ反復解消の最大回数
	static constexpr int MAX_COLLISION_ITERATIONS = 3;
	std::set<std::shared_ptr<KdGameObject>> m_previousHitObjects;

	// --- スピードレベル ---
	SpeedLevel m_level = SpeedLevel::Idle;

	// --- 自動操縦用パラメータ ---
	float m_autoSteerInput = 0.0f;  // 仮想ステアリング入力
	float m_autoAccelInput = 0.0f;  // 仮想アクセル入力（正=アクセル / 負=ブレーキ）

	// ウェイポイント巡回
	std::vector<Math::Vector3> m_waypoints;           // 巡回リスト（ワールド座標）
	int   m_waypointIndex = 0;
	float m_waypointReachRadius = 0.01f;              // 到達とみなす距離

	// 演出で見せたい目標速度レベル
	SpeedLevel m_autoPilotTargetLevel = SpeedLevel::Speed3;
};