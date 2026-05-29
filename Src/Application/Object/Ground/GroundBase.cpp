#include "GroundBase.h"

void GroundBase::Init()
{
	m_model = std::make_shared<KdModelData>();
	m_model->Load("Asset/Models/map_tiles/map_grass.gltf");
	m_model->Load("Asset/Models/map_tiles/map_road_bridge_canal.gltf");

	m_mWorld = Math::Matrix::CreateTranslation(0, 0, -2);

	// 当たり判定を付けたいので、実体化
	m_pCollider = std::make_unique<KdCollider>();

	// モデルの形状で当たり判定を登録
	m_pCollider->RegisterCollisionShape
	(
		"GroundCollision",				// 識別用の名前
		m_model,						// モデルの形で
		KdCollider::Type::TypeGround	// タイプ
	);
}

void GroundBase::DrawLit()
{
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld);
}
