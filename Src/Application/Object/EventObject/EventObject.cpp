#include "EventObject.h"

void EventObject::Init()
{
	m_pCollider = std::make_unique<KdCollider>();

	KdCollider::SphereInfo sphere(0, GetPos(), 10.0f);
	m_pCollider->RegisterCollisionShape
	(
		"EventCollision",
		GetPos(),
		m_radius,
		KdCollider::TypeEvent
	);
}

void EventObject::Update(float dt)
{}

void EventObject::DrawUnLit()
{}
