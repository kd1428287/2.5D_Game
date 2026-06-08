#pragma once

class EventObject : public KdGameObject
{
public:
	EventObject() {};
	EventObject(const std::shared_ptr<KdGameObject>& obj, Math::Vector3 pos, float radius) : m_radius(radius)
	{
		SetParent(obj);
		if (!m_wpParent.expired())
		{
			m_mWorld =
				Math::Matrix::CreateTranslation(pos) *
				Math::Matrix::CreateTranslation(m_wpParent.lock()->GetMatrix().Translation());
		}
	};
	~EventObject()override {};

	void Init()override;
	void Update(float dt)override;
	void DrawUnLit()override;

	void SetParent(const std::shared_ptr<KdGameObject>& obj) { m_wpParent = obj; }

private:
	std::weak_ptr<KdGameObject> m_wpParent;
	float m_radius = 0.0f;
};