#pragma once

enum class BuildingState
{
	None = 0,
	UnBreak,
	Break,
	Erase,
};

enum class BuildingType
{

};

class Building : public KdGameObject
{
public:
	Building() {};
	~Building()override {};

	void Init()override;
	void Update(float dt)override;
	void PostUpdate()override;

	void GenerateDepthMapFromLight()override;
	void DrawLit()override;

protected:
	std::shared_ptr<KdModelData> m_model;

	Math::Vector3 m_pos;


};