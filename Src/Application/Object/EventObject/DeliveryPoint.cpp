#include "DeliveryPoint.h"
#include "../Building/Building.h"
#include "Application/Scene/SceneManager.h"
#include "Application/System/EventBus/Event/Event.h"

DeliveryPoint::DeliveryPoint(const std::shared_ptr<Building>& obj, Math::Vector3 pos, float radius) :
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

void DeliveryPoint::Init()
{
	m_pCollider = std::make_unique<KdCollider>();
	m_pDebugWire = std::make_unique<KdDebugWireFrame>();
	m_model = std::make_shared<KdModelData>();
}

void DeliveryPoint::Update(float dt)
{
	m_deltaTime = dt;
	if (m_wpParent.lock()->GetState() != BuildingState::Unbroken)m_isExpired = true;
}

void DeliveryPoint::PostUpdate()
{
	KdCollider::SphereInfo sphere(KdCollider::TypeEvent, GetPos(), m_radius);
	if (m_isDelivered)return;
	bool isHit = false;
	m_pDebugWire->AddDebugSphere(sphere.m_sphere.Center, sphere.m_sphere.Radius, { 1.0f, 0.0f, 0.0f, 1.0f });
	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		if (obj->Intersects(sphere, nullptr))
		{
			if (m_durationContact <= 0.f)
			{
				GLOBALEVENT.publish(Events::Player::DeliveryPointBegin(shared_from_this()));
			}
			if (m_durationContact < 100.f)
			{
				m_durationContact += 100 * m_deltaTime;
			}
			else
			{
				m_durationContact = 100.f;
				GLOBALEVENT.publish(Events::Player::DeliveryPointCompleted(shared_from_this()));
				m_model = KdAssets::Instance().m_modeldatas.GetData("Asset/Models/Effect/Cardboard.gltf");
				m_isDelivered = true;
			}
			isHit = true;
			break;
		}
	}

	if (!isHit)
	{
		if (m_durationContact > 0.f)
		{
			m_durationContact -= m_deltaTime;
			if (m_durationContact <= 0.f)
			{
				GLOBALEVENT.publish(Events::Player::DeliveryPointEnd(shared_from_this()));
				m_durationContact = 0.f;
			}
		}
		else m_durationContact = 0.f;
	}
}

void DeliveryPoint::DrawUnLit()
{}

void DeliveryPoint::UpdateHitCollision(const std::vector<std::shared_ptr<KdGameObject>>&objList)
{}

void DeliveryPoint::GenerateDepthMapFromLight()
{
	if (!m_isDelivered)return;
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld);
}

void DeliveryPoint::DrawLit()
{
	if (!m_isDelivered)return;
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld);
}

