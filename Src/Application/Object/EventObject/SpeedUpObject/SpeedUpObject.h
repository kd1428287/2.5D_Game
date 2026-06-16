#pragma once

class SpeedUpObject : public KdGameObject
{
public:
	SpeedUpObject() {};
	SpeedUpObject(Math::Vector3 pos) : m_pos(pos) { m_mWorld = Math::Matrix::CreateTranslation(pos); };
	~SpeedUpObject()override {};

	void Init()override;
	void Update(float dt)override;
	void PostUpdate()override;

	void GenerateDepthMapFromLight()override;
	void DrawLit()override;

private:
	std::shared_ptr<KdModelData> m_model;
	float m_rotateRatio = 0.f;
	Math::Vector3 m_pos;

	float m_durationContact = 0.0f;

	bool m_isDelivered = false;

};