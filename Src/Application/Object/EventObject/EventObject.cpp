#include "EventObject.h"
#include "../Building/Building.h"
#include "Application/Scene/SceneManager.h"
#include "Application/System/EventBus/Event/Event.h"

EventObject::EventObject(const std::shared_ptr<Building>& obj, Math::Vector3 pos, float radius) :
	m_radius(radius)
{
	SetParent(obj);
	if (!m_wpParent.expired())
	{
		m_mWorld =
			Math::Matrix::CreateTranslation(pos) *
			Math::Matrix::CreateTranslation(m_wpParent.lock()->GetMatrix().Translation());
	}
};

void EventObject::Init()
{
	m_pCollider = std::make_unique<KdCollider>();

	/*m_pCollider->RegisterCollisionShape
	(
		"EventCollision",
		GetPos(),
		m_radius,
		KdCollider::TypeEvent
	);*/
}

void EventObject::Update(float dt)
{
	m_deltaTime = dt;
}

void EventObject::PostUpdate()
{
	KdCollider::SphereInfo sphere(KdCollider::TypeEvent, GetPos(), m_radius);
	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		if (obj->Intersects(sphere, nullptr))
		{
			
			if (m_durationContact <= 0.f)GLOBALEVENT.publish(Events::Player::EventTouchBegin(shared_from_this()));
			if (m_durationContact <= 0.f)GLOBALEVENT.publish(Events::Player::EventTouchBegin(shared_from_this()));
			if (m_durationContact < 100.f)m_durationContact += m_deltaTime;
			else m_durationContact = 100.f;
		}
		else
		{
			if (m_durationContact > 0.f)
			{
				m_durationContact -= m_deltaTime;
				if (m_durationContact <= 0.f)
				{
					GLOBALEVENT.publish(Events::Player::EventTouchEnd(shared_from_this()));
					m_durationContact = 0.f;
				}
			}
			else m_durationContact = 0.f;
		}
	}
}

void EventObject::DrawUnLit()
{}

void EventObject::UpdateHitCollision(const std::vector<std::shared_ptr<KdGameObject>>&objList)
{}
