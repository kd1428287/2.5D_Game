#pragma once

class Player;

enum class CameraState
{
	Title,
	TitleToGame,
	Game,
	GameToResult,
	Result,
	ResultToTitle
};

class CameraManager : public std::enable_shared_from_this<CameraManager>
{
public:
	CameraManager() {};
	~CameraManager() { Release(); };

	void Release();
	void Init();
	void SetUp(CameraState state);
	void Update(float dt);

	void UpdateTitle(float dt);
	void UpdateTitletoGame(float dt);
	void UpdateGame(float dt);

	void DrawSprite();

	void SetCameraTarget(std::shared_ptr<Player> targetObj) { m_targetObj = targetObj; };
	void SetCameraPos(Math::Vector3 camPos) { m_camPos = camPos; }
	void SetCameraAngleX(float xAng) { m_camAng.x = xAng; }
	void SetCameraAngleY(float yAng) { m_camAng.y = yAng; }
	void SetCameraAngleZ(float zAng) { m_camAng.z = zAng; }

	Math::Vector3 GetCameraPos() { return m_camPos; }
	Math::Vector3 GetCameraAngle() { return m_camAng; }


private:
	void UpdateProjection(float dt);
	void UpdateAngle(const std::shared_ptr<Player>& target,float dt);
	void UpdateDistance(const std::shared_ptr<Player>& target, float dt);

	std::unique_ptr<KdCamera> m_camera = nullptr;
	const Math::Vector3 DEF_DIS = { 0.0f,0.5f,-0.5f };
	Math::Vector3 m_camDis;			// ターゲットからの距離
	Math::Vector3 m_camPos;			// ワールド座標
	Math::Vector3 m_camAng;			// 回転

	float m_projection = 60.0f;		// 視野角
	float m_targetAngle = 15.0f;	// 目標角度
	float m_steeringOffset = 0.0f;	// 旋回オフセット
	float m_speed = 0.0f;			// 追従速度

	float m_shakeStrength = 0.0f; // 現在のシェイク強度
	float m_shakeTime = 0.0f;     // 残りシェイク時間

	std::weak_ptr<Player> m_targetObj = {};

	float m_targetProj = 0.0f;
	Math::Vector3 m_targetPos;
	float m_targetAspectRatio = 0.0f;

	// ブラー用変数
	float m_blurIntensity = 0.f;
	float m_blurMinRange = 0.f;
	float m_blurMaxRange = 0.f;

	// --- TitleToGame 遷移用変数 ---
	float m_transitionProgress = 0.0f;
	const float TRANSITION_TIME = 2.0f; // 遷移にかかる時間（秒）。好みの速度に調整してください
	Math::Vector3 m_startDis;
	Math::Vector3 m_startAng;
	float m_startProj = 60.0f;

	float m_ambientLight = 0.f;

	ScopedSubscriber m_speedSub;
	ScopedSubscriber m_hitSub;
	ScopedSubscriber m_toGameSub;

	CameraState m_state = CameraState::Title;
};