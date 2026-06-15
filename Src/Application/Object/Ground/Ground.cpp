#include "Ground.h"

#include "Application/System/ResourceManager/ResourceManager.h"

void Ground::Init()
{
	// 全てのマップは8*8 のタイル
	m_model = std::make_shared<KdModelData>();

	// 当たり判定を付けたいので、実体化
	m_pCollider = std::make_unique<KdCollider>();

	if (m_type == GroundType::grass)
	{
		m_model = KdAssets::Instance().m_modeldatas.GetData("Asset/Models/map_tiles/map_grass.gltf");
	}
	else
	{
		SetGroundType(m_type);
	}

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
	case (GroundType)0:m_model = KdAssets::Instance().m_modeldatas.GetData("Asset/Models/map_tiles/map_grass.gltf"); break;
	case (GroundType)1:m_model = KdAssets::Instance().m_modeldatas.GetData("Asset/Models/map_tiles/map_road_straight.gltf"); break;
	case (GroundType)2:m_model = KdAssets::Instance().m_modeldatas.GetData("Asset/Models/map_tiles/map_road_corner.gltf"); break;
	case (GroundType)3:m_model = KdAssets::Instance().m_modeldatas.GetData("Asset/Models/map_tiles/map_road_tjunction.gltf"); break;
	case (GroundType)4:m_model = KdAssets::Instance().m_modeldatas.GetData("Asset/Models/map_tiles/map_road_crossing.gltf"); break;
	case (GroundType)5:m_model = KdAssets::Instance().m_modeldatas.GetData("Asset/Models/map_tiles/map_road_end.gltf"); break;
	case (GroundType)6:m_model = KdAssets::Instance().m_modeldatas.GetData("Asset/Models/map_tiles/map_road_bridge_canal.gltf"); break;
		
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
