#include "Building.h"
#include "../Character/Player/Player.h"
#include "../Character/Player/Player.h"

void Building::Init()
{
	m_model = std::make_shared<KdModelData>();
	m_model = KdAssets::Instance().m_modeldatas.GetData("Asset/Models/map_tiles/building.gltf");

	// 当たり判定を付けたいので、実体化
	m_pCollider = std::make_unique<KdCollider>();

	m_breakCount = 5.0f;

	m_subscriber.push_back(
		GLOBALEVENT.subscribe<Events::Player::OnHit>([this](const Events::Player::OnHit& e)
		{
			if (shared_from_this() != e.m_other.lock())return;
			auto type = Events::Player::HitResult::HitResultType::Bounced;
			std::shared_ptr<Player> player = std::dynamic_pointer_cast<Player>(e.m_me.lock());
			if (!player)return;
			if ((int)player->GetSpeedLevel() >= m_breakLevel && player->GetSpeedLevel() != SpeedLevel::Clash)
			{
				Break(e.m_result);
				type = Events::Player::HitResult::HitResultType::Destroyed;
			}
			GLOBALEVENT.publish(Events::Player::HitResult(type, GetPos(), (int)player->GetSpeedLevel()));
		}));

	//m_dir = 180.f;
	m_scale = 1.15f;
	if (m_breakLevel == 99)m_scale = 1.3f;
	SetDir(m_dir);
	SetBreakLevel(m_breakLevel);
	SetModel(m_type);

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
			fragVelocities[i].direction.y -= 1.5f * dt; // 重力
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

		//// dissolve
		//m_buildingDissolve += 2 * dt;
		//if (m_buildingDissolve > 1.0f)m_buildingDissolve = 1.0f;

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

void Building::GenerateDepthMapFromLight()
{
	
	if (m_state == BuildingState::Unbroken)KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld, m_color);
	else KdShaderManager::Instance().m_StandardShader.DrawModel(*m_fragmentModel, m_mWorld, m_color);

	/*if (m_state == BuildingState::Broken) {
		KdShaderManager::Instance().m_StandardShader.SetDissolve(m_buildingDissolve);
	}
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld, m_color);
	if (m_state != BuildingState::Broken) return;
	KdShaderManager::Instance().m_StandardShader.SetDissolve(0);
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_fragmentModel, m_mWorld, m_color); */
}

void Building::DrawLit()
{
	if (m_state == BuildingState::Unbroken)KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld, m_color);
	else KdShaderManager::Instance().m_StandardShader.DrawModel(*m_fragmentModel, m_mWorld, m_color);

	//if (m_state == BuildingState::Broken) {
	//	KdShaderManager::Instance().m_StandardShader.SetDissolve(m_buildingDissolve);
	//}
	//KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld, m_color);
	//if (m_state != BuildingState::Broken) return;
	//KdShaderManager::Instance().m_StandardShader.SetDissolve(0);
	//KdShaderManager::Instance().m_StandardShader.DrawModel(*m_fragmentModel, m_mWorld, m_color);
}

void Building::SetPos(Math::Vector3 pos)
{
	m_pos = pos;
	m_mWorld =
		Math::Matrix::CreateScale(m_scale) *
		Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_dir)) *
		Math::Matrix::CreateTranslation(m_pos);
}

void Building::SetDir(float dir)
{
	m_dir = dir;
	SetPos(m_pos);
}

void Building::Break(KdCollider::CollisionResult result)
{
	if (m_state != BuildingState::Unbroken)return;

	GLOBALEVENT.publish(Events::Else::CreateObjectEvent("Exprosion", GetPos() + Math::Vector3(0,0.1f,0)));
	// 状態遷移
	m_state = BuildingState::Broken;

	// 当たり判定オフ
	m_pCollider->SetEnable(KdCollider::Type::TypeBump, false);

	// 破片ノード取得
	m_fragmentModel = std::make_shared<KdModelWork>();
	m_fragmentModel->SetModelData(KdAssets::Instance().m_modeldatas.GetData("Asset/Models/map_tiles/building_fragment.gltf"));
	
	auto& nodes = m_fragmentModel->WorkNodes(); // 可変ノードリストを取得
	fragVelocities.resize(nodes.size());

	// 各ノードの移動ベクトル & 回転速度セット
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		// 初期位置を現在のモデルのローカル行列から取得
		fragVelocities[i].position = nodes[i].m_localTransform.Translation();
		fragVelocities[i].rotation = Math::Vector3(
			KdRandom::GetFloat(0.f, 360.0f),
			KdRandom::GetFloat(0.f, 360.0f),
			KdRandom::GetFloat(0.f, 360.0f)
		);
		fragVelocities[i].speed = 1.5f;

		// 爆発のように四方に飛び散るランダムな速度を設定 (例)
		fragVelocities[i].direction = Math::Vector3(
			KdRandom::GetFloat(-1.0f, 1.0f),
			KdRandom::GetFloat(0.5f, 0.85f), // 少し上に跳ね上がる
			KdRandom::GetFloat(-1.0f, 1.0f)
		);
		fragVelocities[i].direction += result.m_hitDir * result.m_overlapDistance * -1;
		fragVelocities[i].direction.Normalize();
		
		// ランダムな回転速度
		fragVelocities[i].angularVelocity = Math::Vector3(
			KdRandom::GetFloat(-1.0f, 1.0f),
			KdRandom::GetFloat(-1.0f, 1.0f),
			KdRandom::GetFloat(-1.0f, 1.0f)
		);
	}
}

void Building::SetBreakLevel(int level)
{
	switch (level)
	{
	case 0: m_color = { 1.f,1.f,1.f,1.f }; break;
	case 1:  m_model = KdAssets::Instance().m_modeldatas.GetData("Asset/Models/map_tiles/building1.gltf");  break;

	case 2: m_color = { 0.8f,0.8f,0.2f,1.f }; break;

	case 3: m_color = { 0.6f,0.6f,0.8f,1.f }; break;	// 黄

	case 4: m_color = { 0.6f,0.8f,0.6f,1.f }; break;	// オレンジ

	case 5: m_color = { 0.8f,0.6f,0.6f,1.f }; break;	// 赤

	case 6: m_color = { 0.6f,0.4f,0.6f,1.f }; break;	// 紫

	case 99: m_color = { 0.2f,0.2f,0.2f,1.f }; break;
	default:
		break;
	}
	m_breakLevel = level;

	//switch (level)
	//{
	//case 0: m_color = { 1.f,1.f,1.f,1.f }; break;
	//case 1:  m_model = KdAssets::Instance().m_modeldatas.GetData("Asset/Models/map_tiles/building1.gltf");  break;

	//case 2: m_color = { 0.8f,0.8f,0.2f,1.f }; break;

	//case 3: m_color = { 0.7f,0.8f,0.0f,1.f }; break;	// 黄

	//case 4: m_color = { 0.8f,0.6f,0.0f,1.f }; break;	// オレンジ

	//case 5: m_color = { 0.8f,0.6f,0.6f,1.f }; break;	// 赤

	//case 6: m_color = { 0.6f,0.3f,0.6f,1.f }; break;	// 紫

	//case 99: m_color = { 0.2f,0.2f,0.2f,1.f }; break;
	//default:
	//	break;
	//}
	//m_breakLevel = level;
}

void Building::SetModel(BuildingType type)
{
	switch (type)
	{
	case BuildingType::Building:
		m_model = KdAssets::Instance().m_modeldatas.GetData("Asset/Models/map_tiles/building.gltf");
		break;
	case BuildingType::HouseCountry:
		m_model = KdAssets::Instance().m_modeldatas.GetData("Asset/Models/map_tiles/house_country.gltf");
		break;
	case BuildingType::House:
		m_model = KdAssets::Instance().m_modeldatas.GetData("Asset/Models/map_tiles/collection_point.gltf");
		break;
	default:
		break;
	}
}
