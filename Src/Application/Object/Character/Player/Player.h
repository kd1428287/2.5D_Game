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
	void Update(float dt)override;
	void PostUpdate()override;
	
	void GenerateDepthMapFromLight()override;
	void DrawLit()override;

private:
	void ActiveInput();

private:
	std::shared_ptr<KdSquarePolygon> m_polygon = nullptr;

	Math::Vector3 m_pos;
	PlayerState m_state = PlayerState::Idle;
};