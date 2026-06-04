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

private:
	void ActiveInput();
	void UpdateMove(float dt);

private:
	std::shared_ptr<KdSquarePolygon> m_polygon = nullptr;
	std::shared_ptr<KdModelData> m_model = nullptr;

	Math::Vector3 m_pos;
	Math::Vector3 m_angle;
	Math::Vector3 m_moveVec;
	float m_speed = 0.0f;				//スピード
	float m_gravity = 0.0f;				//重力

	float m_acceleration = 0.0f;		//加速力
	float m_turningForce = 0.0f;		//旋回力
	SpeedLevel m_level = SpeedLevel::Idle;
};