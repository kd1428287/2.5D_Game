#pragma once

enum class PlayerState
{
	Idle,
	Walk_start,
	Walk_middle,
	Walk_end,
	Attack_start,
	Attack_middle,
	Attack_end,
};

class Player : public KdGameObject
{
public:
	Player() {};
	~Player()override {};

	void Init()override;
	void PreUpdate()override;
	void Update(float dt)override;
	void PostUpdate()override;
	
	void GenerateDepthMapFromLight()override;
	void DrawLit()override;

private:
	void ActiveInput();

private:
	std::shared_ptr<KdSquarePolygon> m_polygon = nullptr;
	std::shared_ptr<KdModelData> m_model = nullptr;

	Math::Vector3 m_pos;
	Math::Vector3 m_moveVec;
	float m_speed = 0.0f;
	PlayerState m_state = PlayerState::Idle;
};