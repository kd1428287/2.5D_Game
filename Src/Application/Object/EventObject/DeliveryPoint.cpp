#include "DeliveryPoint.h"
#include "../Building/Building.h"
#include "Application/Scene/SceneManager.h"
#include "Application/System/EventBus/Event/Event.h"
#include "../../System/ConvertScreen/ConvertScreen.h"

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
	m_model = std::make_shared<KdModelData>();
	m_model = KdAssets::Instance().m_modeldatas.GetData("Asset/Models/Effect/Circle.gltf");
	m_texture = std::make_shared<KdTexture>();
	m_texture = KdAssets::Instance().m_textures.GetData("Asset/Textures/Effect/Arrow.png");

	m_startSub = GLOBALEVENT.subscribe<Events::Else::GameStart>([this](const Events::Else::GameStart& e)
		{
			m_isSpriteDraw = true;
		});
}

void DeliveryPoint::Update(float dt)
{
	m_deltaTime = dt;
	if (m_wpParent.lock()->GetState() != BuildingState::Unbroken)
	{
		GLOBALEVENT.publish(Events::Player::DeliveryPointDeleted(shared_from_this()));
		m_isExpired = true;
	}

	// 放物線アニメーション
	if (m_isAnimating)
	{
		m_animTime += dt;
		float t = m_animTime / m_animDuration;

		if (t >= 1.0f)
		{
			t = 1.0f;
			m_isAnimating = false;
		}

		Math::Vector3 goal = GetPos();
		Math::Vector3 lerpPos = Math::Vector3::Lerp(m_cardboardPos, goal, t);
		float arc = std::sin(t * DirectX::XM_PI) * m_arcHeight;
		lerpPos.y += arc;
		m_cardboardWorldPos = lerpPos;

		// スケール補間
		m_cardboardScale = 0.2f + 0.8f * t;
	}

	if (m_isDelivered || !m_isSpriteDraw)return;
	ConvertData data;
	data.mat = 
		Math::Matrix::CreateTranslation(m_mWorld.Translation() + m_arrowDistance);
	data.tex = m_texture;
	ConvertScreen::Instance().RequestConvertScreen(data);

}

void DeliveryPoint::PostUpdate()
{
	KdCollider::SphereInfo sphere(KdCollider::TypeEvent, GetPos(), m_radius);
	if (m_isDelivered)return;
	bool isHit = false;
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
				m_cardboardPos = obj->GetPos();   // 開始位置（既存）
				m_isAnimating = true;            // アニメーション開始
				m_animTime = 0.0f;
				m_cardboardWorldPos = m_cardboardPos;  // 初期描画位置
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

void DeliveryPoint::DrawLit()
{
	if (!m_isDelivered) return;

	// アニメーション中は飛翔位置、完了後は自身の位置で描画
	Math::Matrix drawMat = m_isAnimating
		? Math::Matrix::CreateScale(m_cardboardScale) *
		Math::Matrix::CreateTranslation(m_cardboardWorldPos)
		: m_mWorld;

	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, drawMat);
}

void DeliveryPoint::GenerateDepthMapFromLight()
{
	if (!m_isDelivered) return;

	Math::Matrix drawMat = m_isAnimating
		? Math::Matrix::CreateScale(m_cardboardScale) *
		Math::Matrix::CreateTranslation(m_cardboardWorldPos)
		: m_mWorld;

	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, drawMat);
}
void DeliveryPoint::DrawBright()
{
	if (m_isDelivered)return;
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld);
}

void DeliveryPoint::DrawSprite()
{
}

