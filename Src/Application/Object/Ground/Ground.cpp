#include "Ground.h"

void Ground::Init()
{
	// 全てのマップは8*8 のタイル
	m_model = std::make_shared<KdModelData>();

	m_mWorld = 
		Math::Matrix::CreateTranslation(0, 0, 0);

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

void Ground::DrawLit()
{
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld);
}

void Ground::SetGroundType(GroundType type)
{
	switch (type)
	{
	case (GroundType)0:m_model->Load("Asset/Models/map_tiles/map_grass.gltf");
		break;
	default:
		break;
	}

	m_pCollider->RegisterCollisionShape
	(
		"GroundCollision",				// 識別用の名前
		m_model,						// モデルの形で
		KdCollider::Type::TypeGround	// タイプ
	);
}
