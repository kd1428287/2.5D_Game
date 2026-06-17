#include "SpeedUpObject.h"
#include "../../../Scene/SceneManager.h"

void SpeedUpObject::Init()
{
	m_model = std::make_shared<KdModelData>();
	m_model = KdAssets::Instance().m_modeldatas.GetData("Asset/Models/Effect/SpeedUp.gltf");
	
	m_pDebugWire = std::make_unique<KdDebugWireFrame>();
}

void SpeedUpObject::Update(float dt)
{
	// 浮遊タイマーを進める
	m_floatTime += dt;

	// Y軸回転角度を更新
	m_rotateAngle += m_rotateSpeed * dt;
	if (m_rotateAngle >= 360.0f) m_rotateAngle -= 360.0f;

	// 現在地点を基準にサイン波で上下移動
	float offsetY = std::sin(m_floatTime * m_floatSpeed) * m_floatAmplitude;

	// ワールド行列を再計算（回転 × 平行移動）
	Math::Matrix mRot = Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_rotateAngle));
	Math::Matrix mTrans = Math::Matrix::CreateTranslation(m_pos + Math::Vector3(0.0f, offsetY, 0.0f));
	m_mWorld = mRot * mTrans;
}

void SpeedUpObject::PostUpdate()
{
	KdCollider::SphereInfo sphere(KdCollider::TypeEvent, GetPos(), m_radius);
	if (m_isDelivered)return;
	bool isHit = false;
	//m_pDebugWire->AddDebugSphere(sphere.m_sphere.Center, sphere.m_sphere.Radius, { 1.0f, 0.0f, 0.0f, 1.0f });
	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		if (obj->Intersects(sphere, nullptr))
		{
			GLOBALEVENT.publish(Events::Player::GetSpeedUp(shared_from_this()));
			m_isExpired = true;
		}
	}
}

void SpeedUpObject::GenerateDepthMapFromLight()
{
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld);
}

void SpeedUpObject::DrawLit()
{
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld);
}