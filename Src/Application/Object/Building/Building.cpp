#include "Building.h"

void Building::Init()
{
	m_model = std::make_shared<KdModelData>();
	m_model->Load("Asset/Models/");

	// 当たり判定を付けたいので、実体化
	m_pCollider = std::make_unique<KdCollider>();

	UINT type = KdCollider::Type::TypeGround & KdCollider::Type::TypeBump;
	// モデルの形状で当たり判定を登録
	m_pCollider->RegisterCollisionShape
	(
		"BuildingCollision",			// 識別用の名前
		m_model,						// モデルの形で
		type							// タイプ
	);
}

void Building::Update(float dt)
{}

void Building::GenerateDepthMapFromLight()
{}

void Building::DrawLit()
{}
