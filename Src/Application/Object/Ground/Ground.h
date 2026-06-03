#pragma once

enum class GroundType
{
	grass = 0,

};
class Ground : public KdGameObject
{
public:
	Ground() {};
	~Ground()override {};

	void Init()override;
	void DrawLit()override;
	void SetGroundType(GroundType type);
private:
	std::shared_ptr<KdModelData> m_model;
};