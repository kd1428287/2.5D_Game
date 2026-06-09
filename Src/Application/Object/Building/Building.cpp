#include "Building.h"
#include "../Character/Player/Player.h"

void Building::Init()
{
	m_model = std::make_shared<KdModelData>();
	m_model = RESOURCE.GetModel("Asset/Models/map_tiles/building.gltf");

	// 当たり判定を付けたいので、実体化
	m_pCollider = std::make_unique<KdCollider>();

	m_breakCount = 5.0f;
	UINT type = KdCollider::Type::TypeGround | KdCollider::Type::TypeBump;
	// モデルの形状で当たり判定を登録
	m_pCollider->RegisterCollisionShape
	(
		"BuildingCollision",			// 識別用の名前
		m_model,						// モデルの形で
		type							// タイプ
	);
}

void Building::Update(float dt)
{
	

	switch (m_state)
	{
	case BuildingState::None:
		break;
	case BuildingState::Unbroken:
		break;
	case BuildingState::Broken:

	{
		std::vector<KdModelWork::Node>& nodes = m_fragmentModel->WorkNodes();

		for (size_t i = 0; i < nodes.size(); ++i)
		{
			// 重力や速度の計算
			fragVelocities[i].direction.y -= 1.0f * dt; // 重力
			fragVelocities[i].direction.Normalize();
			fragVelocities[i].position += fragVelocities[i].direction * fragVelocities[i].speed * dt;
			

			fragVelocities[i].rotation += fragVelocities[i].angularVelocity * dt;

			// 新しい移動・回転行列を作成
			Math::Matrix matRot = Math::Matrix::CreateFromYawPitchRoll(
				fragVelocities[i].rotation.y,
				fragVelocities[i].rotation.x,
				fragVelocities[i].rotation.z
			);
			Math::Matrix matTrans = Math::Matrix::CreateTranslation(fragVelocities[i].position);

			Math::Matrix matScale = Math::Matrix::CreateScale(1);

			// 破片のローカル行列を直接書き換える
			nodes[i].m_localTransform = matScale * matRot * matTrans;
		}

		// 3. 最後に必ず行列の再計算を行う
		m_fragmentModel->CalcNodeMatrices();

		m_breakCount -= dt;
		if (m_breakCount < 0.0f)m_state = BuildingState::Erase;
	}
		break;
	case BuildingState::Erase:
		m_isExpired = true;
		break;
	default:
		break;
	}
}

void Building::PostUpdate()
{}

void Building::GenerateDepthMapFromLight()
{
	if (m_state == BuildingState::Unbroken)KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld);
	else KdShaderManager::Instance().m_StandardShader.DrawModel(*m_fragmentModel, m_mWorld);
}

void Building::DrawLit()
{
	if (m_state == BuildingState::Unbroken)KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld);
	else KdShaderManager::Instance().m_StandardShader.DrawModel(*m_fragmentModel, m_mWorld);
}

bool Building::OnHit(const std::shared_ptr<KdGameObject>& obj, KdCollider::CollisionResult result)
{
	std::shared_ptr<Player> player = std::dynamic_pointer_cast<Player>(obj);
	if (player)
	{
		if ((int)player->GetSpeedLevel() >= m_breakLevel && player->GetSpeedLevel() != SpeedLevel::Clash)
		{
			Break(result);
			return true;
		}
		else return false;
	}
	else return false;
}

void Building::Break(KdCollider::CollisionResult result)
{
	if (m_state != BuildingState::Unbroken)return;
	m_state = BuildingState::Broken;
	m_pCollider->SetEnable(KdCollider::Type::TypeBump, false);

	m_fragmentModel = std::make_shared<KdModelWork>();
	std::shared_ptr<KdModelData> fragment = nullptr;
	fragment = RESOURCE.GetModel("Asset/Models/map_tiles/building_fragment.gltf");
	if(fragment)m_fragmentModel->SetModelData(fragment);

	auto& nodes = m_fragmentModel->WorkNodes(); // 可変ノードリストを取得
	fragVelocities.resize(nodes.size());

	
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		// 初期位置を現在のモデルのローカル行列から取得
		fragVelocities[i].position = nodes[i].m_localTransform.Translation();
		fragVelocities[i].rotation = Math::Vector3::Zero;
		fragVelocities[i].speed = 15.0f;

		// 爆発のように四方に飛び散るランダムな速度を設定 (例)
		fragVelocities[i].direction = Math::Vector3(
			KdRandom::GetFloat(-1.0f, 1.0f),
			KdRandom::GetFloat(0.2f, 0.85f), // 少し上に跳ね上がる
			KdRandom::GetFloat(-1.0f, 1.0f)
		);
		fragVelocities[i].direction += result.m_hitDir * -1;
		fragVelocities[i].direction.Normalize();
		
		// ランダムな回転速度
		fragVelocities[i].angularVelocity = Math::Vector3(
			KdRandom::GetFloat(-1.0f, 1.0f),
			KdRandom::GetFloat(-1.0f, 1.0f),
			KdRandom::GetFloat(-1.0f, 1.0f)
		);
	}
}
