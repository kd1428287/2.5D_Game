#pragma once

class GroundBase : public KdGameObject
{
public:
	GroundBase() {};
	~GroundBase()override {};

	void Init()override;
	void DrawLit()override;

	//void SetPos(Math::Vector3 pos) { m_pos = pos; };

private:
	std::shared_ptr<KdModelData> m_model;

	Math::Vector3 m_pos;
};