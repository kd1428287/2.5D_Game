#pragma once

class BaseEnemy : public KdGameObject
{
public:
	BaseEnemy() {};
	~BaseEnemy()override {};

	void Init()override;
	void Update(float dt)override;

	void GenerateDepthMapFromLight()override;
	void DrawLit()override;
};