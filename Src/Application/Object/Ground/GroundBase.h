#pragma once

class Ground : public KdGameObject
{
public:
	Ground() {};
	~Ground()override {};

	void Init()override;
	void DrawLit()override;

	void SetPos(Math::Vector3 pos) { m_pos = pos; };

private:
	std::shared_ptr<KdModelData> m_model;

	Math::Vector3 m_pos;
};