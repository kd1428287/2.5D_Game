#pragma once

enum class GroundType
{
	grass = 0,
	a,
	b,
	c,
	d,

};
class Ground : public KdGameObject
{
public:
	Ground() {};
	Ground(Math::Vector3 pos) 
	{
		m_mWorld = Math::Matrix::CreateTranslation(pos);
	};
	Ground(Math::Vector3 pos, int type) :m_type((GroundType)type)
	{
		m_mWorld = 
			Math::Matrix::CreateScale(0.100001f) *	// そのままだとタイルの狭間に落ちてしまうことがある	
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