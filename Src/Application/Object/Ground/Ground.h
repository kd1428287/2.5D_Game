#pragma once

enum class GroundType
{
	grass = 0,
	road_straight,
	road_carb,
	road_junction,
	road_cross,
	road_end,

};
class Ground : public KdGameObject
{
public:
	Ground() {};
	Ground(Math::Vector3 pos) 
	{
		m_mWorld = Math::Matrix::CreateTranslation(pos);
	};
	Ground(Math::Vector3 pos, float dir,int type) :m_type((GroundType)type)
	{
		m_mWorld = 
			Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(dir)) * 
			Math::Matrix::CreateTranslation(pos);
	};
	~Ground()override {};

	void Init()override;
	void DrawLit()override;
	void SetGroundType(GroundType type);
private:
	std::shared_ptr<KdModelData> m_model;
	GroundType m_type = (GroundType)0;
};