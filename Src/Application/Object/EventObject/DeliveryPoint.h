#pragma once

class Building;

class DeliveryPoint :public KdGameObject
{
public:
	DeliveryPoint() {};
	DeliveryPoint(const std::shared_ptr<Building>& obj, Math::Vector3 pos, float radius);
	~DeliveryPoint()override {};

	void Init()override;
	void Update(float dt)override;
	void PostUpdate()override;
	void DrawUnLit()override;

	void SetParent(const std::shared_ptr<Building>& obj) { m_wpParent = obj; }

private:
	void UpdateHitCollision(const std::vector<std::shared_ptr<KdGameObject>>& objList);

	std::weak_ptr<Building> m_wpParent;
	float m_radius = 0.0f;
	float m_deltaTime = 0.0f;

	//static const float MAX_DURATION = 100.0f;
	float m_durationContact = 0.0f;

};