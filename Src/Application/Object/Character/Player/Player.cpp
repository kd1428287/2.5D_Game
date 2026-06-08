#include "Player.h"

#include "Application/System/InputManager/InputManager.h"
#include "Application/System/CameraManager/CameraManager.h"
#include "Application/Scene/SceneManager.h"

void Player::Init()
{
	// デバッグワイヤ
	m_pDebugWire = std::make_unique<KdDebugWireFrame>();

	// 当たり判定
	m_pCollider = std::make_unique<KdCollider>();

	// モデル
	m_model = std::make_shared<KdModelData>();
	m_model = RESOURCE.GetModel("Asset/Models/car_van/car_van.gltf");
	
	//m_pos = { 0,5,0 };
	m_speed = 0.0f;
	m_angle = { 0,0,0 };
	m_level = SpeedLevel::Idle;

	ChangeSpeedLevel(SpeedLevel::Idle);

	//m_acceleration = 0.05f;
	m_acceleration = 50.0f;

	m_pCollider->RegisterCollisionShape(
		"PlayerCollision",
		m_model,
		KdCollider::Type::TypeEvent & KdCollider::Type::TypeBump
	);
}

void Player::PreUpdate()
{
	m_amountMove = Math::Vector3::Zero;
}

void Player::Update(float dt)
{
	UpdateMove(dt);
	if (m_level == SpeedLevel::Clash)
	{
		m_clashCount -= dt;
		if (m_clashCount <= 0.0f)
		{
			ChangeSpeedLevel(SpeedLevel::Idle); 
			m_clashCount = 0.0f;
		}
	}
}

void Player::PostUpdate()
{
	Math::Matrix rotMat =
		Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_angle.y)) *
		Math::Matrix::CreateRotationX(DirectX::XMConvertToRadians(m_angle.x));

	// 全オブジェクトの中から自機から一定距離内のオブジェクトを取得
	std::vector<std::shared_ptr<KdGameObject>> objList;
	Math::Vector3 length;
	float radius = m_gravity + 15.0f;
	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		length = m_pos + m_amountMove - obj->GetPos();
		if (length.Length() < radius)objList.push_back(obj);
	}

	// 当たる側の処理
	// レイ判定用の変数を設定
	KdCollider::RayInfo ray;
	// レイの発射位置を設定
	ray.m_pos = m_pos + m_amountMove;
	// ちょっと上からの位置にする
	ray.m_pos.y += 0.1f;
	// 段差の許容範囲を設定
	float enableStepHigh = 0.2f;
	ray.m_pos.y += enableStepHigh;
	// レイの発射方向を設定
	ray.m_dir = { 0,-1,0 };
	// レイの長さを設定
	ray.m_range = m_gravity + enableStepHigh;
	// 当たり判定を行いたいタイプを設定
	ray.m_type = KdCollider::TypeGround;

	// デバッグ表示
	m_pDebugWire->AddDebugLine(ray.m_pos, ray.m_dir, ray.m_range);

	// レイに当たったオブジェクト情報を格納するリストを用意
	std::list<KdCollider::CollisionResult> retRayList;

	// 判定
	for (auto& obj : objList)
	{
		//		↓当たり判定を行う関数
		obj->Intersects(ray, &retRayList);
	}

	// レイに当たったリストから一番近いオブジェクトを探す
	float maxOverLap = 0.0f;
	Math::Vector3 hitPos;
	bool hit = false;
	for (auto& ret : retRayList)
	{
		// レイを遮断し、オーバーした長さが一番長いものを探す
		if (maxOverLap < ret.m_overlapDistance)
		{
			maxOverLap = ret.m_overlapDistance;
			hitPos = ret.m_hitPos;
			hit = true;
		}
	}

	if (hit)
	{
		// 当たっていたらプレイヤー座標を更新
		if (m_amountMove.y + m_pos.y < hitPos.y)m_amountMove.y = 0.0f;
		m_gravity = 0.0f;
	}

	
	// 球(スフィア)判定=========

	const int SPHERE_NUM = 3;
	float zOffset[SPHERE_NUM] = { 0.7f,0.0f,-0.7f };

	KdCollider::SphereInfo sphere[SPHERE_NUM];
	KdCollider::SphereInfo mainSphere;
	mainSphere.m_sphere.Center = m_pos + m_amountMove;
	mainSphere.m_sphere.Radius = 1.2f;
	mainSphere.m_type = KdCollider::TypeBump;

	for (int i = 0; i < SPHERE_NUM; ++i)
	{
		
		// ローカルのズレ(Z方向)を、車の現在の向き(回転行列)に合わせてワールド空間のズレに変換
		Math::Vector3 offset = { 0.0f, 0.0f, zOffset[i] };
		Math::Vector3 rotatedOffset = Math::Vector3::TransformNormal(offset, rotMat);

		// 球の中心座標 ＝ 車の中心座標 ＋ 高さ調整 ＋ 向きを考慮した前後のズレ
		sphere[i].m_sphere.Center = m_pos + m_amountMove + Math::Vector3(0.0f, 0.6f, 0.0f) + rotatedOffset;

		// 車幅の半分くらいを半径に設定
		sphere[i].m_sphere.Radius = 0.5f;
		sphere[i].m_type = /*KdCollider::TypeGround |*/ KdCollider::TypeBump; 

		// デバッグ描画（赤いワイヤーフレームで球を描画）
		m_pDebugWire->AddDebugSphere(sphere[i].m_sphere.Center, sphere[i].m_sphere.Radius, { 1.0f, 0.0f, 0.0f, 1.0f });
	}

	std::list<KdCollider::CollisionResult> retSphereList;

	// 当たり判定

	for (auto& obj : objList)
	{
		for (int i = 0; i < SPHERE_NUM; i++)
		{
			if (obj->Intersects(sphere[i], &retSphereList))
			{
				//obj->Intersects(mainSphere, &retSphereList);
				auto ret = retSphereList.back();
				if (!obj->OnHit(shared_from_this(), ret))
				{
					if (m_level != SpeedLevel::Clash);
					ChangeSpeedLevel(SpeedLevel::Clash);

					if (std::abs(m_speed) < 2.0f)m_speed = 0.0f;
					m_speed *= -0.5;
					m_clashCount = 0.0;
					m_steering = 0.0f;
				}
				//break;
			}
		}
	}

	maxOverLap = 0.0f;
	hit = false;
	Math::Vector3 totalPushOut = Math::Vector3::Zero; // 全ての押し出しベクトルの合計

	for (auto& ret : retSphereList)
	{
		Math::Vector3 dir = ret.m_hitDir;

		// 下向きの押し出し（床からの過剰な反発など）を無効化
		if (dir.y < 0.0f) dir.y = 0.0f;

		// 安全のため正規化（ゼロ除算防止）
		if (dir.LengthSquared() > 0.0f)
		{
			dir.Normalize();

			// 全てのめり込みベクトルを加算する
			// ※ 0.75f のような妥協はせず、1.0f (100%) 押し出します
			totalPushOut += dir * ret.m_overlapDistance;
			hit = true;
		}
	}

	if (hit)
	{
		// 合成された押し出しベクトルを適用してプレイヤーの座標を更新
		m_amountMove += totalPushOut;

		// --- 追加のプロテクニック（壁滑り） ---
		// 壁に押し出された後、まだ車が壁に向かおうとする速度(m_moveVecによる移動)が残っていると、
		// 常に壁に向かって進み続けてしまいガタつきます。
		// 押し出し方向に逆らう移動成分をカットすることで、壁に沿って綺麗に滑るようになります。

		Math::Vector3 pushDir = totalPushOut;
		pushDir.Normalize();

		// 移動ベクトル(m_amountMove)のうち、壁に垂直にぶつかろうとする成分を削る
		float dotResult = m_amountMove.Dot(pushDir);
		if (dotResult < 0.0f)
		{
			m_amountMove -= pushDir * dotResult;
		}
	}

	// ----------------------------------------------------
	// 全ての押し出しが終わった後の最終的な m_pos で m_mWorld を確定させる
	// ----------------------------------------------------

	m_pos += m_amountMove;

	m_mWorld = 
		rotMat * 
		Math::Matrix::CreateTranslation(m_pos);
}

void Player::GenerateDepthMapFromLight()
{
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld);
}

void Player::DrawLit()
{
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld);
}

void Player::ChangeSpeedLevel(SpeedLevel level)
{
	if (m_level == level)return;
	m_level = level;

	switch (m_level)
	{
	case SpeedLevel::Idle:
	case SpeedLevel::Speed1: 
		m_maxSpeed = 8.f;
		m_minSpeed = -8.0f;
		break;
	case SpeedLevel::Speed2: 
		m_maxSpeed = 11.f; 
		m_minSpeed = 8.0f;
		break;
	case SpeedLevel::Speed3: 
		m_maxSpeed = 15.f; 
		m_minSpeed = 11.0f;
		break;
	case SpeedLevel::Speed4: 
		m_maxSpeed = 20.f; 
		m_minSpeed = 15.0f;
		break;
	case SpeedLevel::Speed5: 
		m_maxSpeed = 30.f; 
		m_minSpeed = 20.0f;
		break;
	case SpeedLevel::Clash:  
		m_maxSpeed = 0.f;
		m_minSpeed = -8.0f;
		break;
	default: break;
	}

	m_speed = std::max(m_speed, m_minSpeed);
}

void Player::ActiveInput()
{
}

void Player::UpdateMove(float dt)
{
	float steerInput = 0.0f;
	if (InputManager::Instance().IsPressed(VK_LEFT))  steerInput = -1.0f;
	if (InputManager::Instance().IsPressed(VK_RIGHT)) steerInput = 1.0f;

	if (steerInput != 0.0f)
	{
		// 入力がある場合はハンドルを切る
		m_steering += steerInput * m_steerSpeed * dt;
	}
	else
	{
		// 入力がない場合は自動でハンドルをセンターに戻す（遊びを持たせる）
		if (m_steering > 0.0f) {
			m_steering -= m_steerSpeed * dt;
			if (m_steering < 0.0f) m_steering = 0.0f;
		}
		else if (m_steering < 0.0f) {
			m_steering += m_steerSpeed * dt;
			if (m_steering > 0.0f) m_steering = 0.0f;
		}
	}

	// 最大切れ角で制限（クランプ）
	m_steering = std::clamp(m_steering, -m_maxSteerAngle, m_maxSteerAngle);


	// ==========================================
	// 2. アクセルとブレーキの操作
	// ==========================================
	if (m_level != SpeedLevel::Clash)
	{
		if (InputManager::Instance().IsPressed(VK_UP))
		{
			m_speed += m_acceleration * dt;
		}
		if (InputManager::Instance().IsPressed(VK_DOWN))
		{
			// ブレーキ（あるいはバック）
			m_speed -= m_acceleration * dt;
		}
	}


	if (m_speed < m_minSpeed && m_level != SpeedLevel::Idle && m_level != SpeedLevel::Clash)ChangeSpeedLevel((SpeedLevel)((int)m_level - 1));
	if (m_level == SpeedLevel::Idle && m_speed > std::abs(0.5f))ChangeSpeedLevel(SpeedLevel::Speed1);

	// 速度レベル毎の上限処理（既存のまま）
	if (m_speed > m_maxSpeed)m_speed = m_maxSpeed;
	if (m_speed < m_minSpeed)m_speed = m_minSpeed;


	// ==========================================
	// 3. 車の向き（ヘディング）の更新 (バイシクルモデル)
	// ==========================================
	// 車が動いている時だけ向きが変わるようにする
	if (std::abs(m_speed) > 0.001f)
	{
		// C++の標準数学関数はラジアンを要求するため変換
		float steerRad = DirectX::XMConvertToRadians(m_steering);

		// 角速度 ω = (V * tan(δ)) / L
		float angularVelocity = (m_speed * std::tan(steerRad)) / m_wheelBase;

		m_angle.y += DirectX::XMConvertToDegrees(angularVelocity * dt);
	}


	// ==========================================
	// 4. 移動ベクトルの計算と位置の更新
	// ==========================================
	float pitch_rad = DirectX::XMConvertToRadians(m_angle.x);
	float yaw_rad = DirectX::XMConvertToRadians(m_angle.y); // すでに更新済みの向きを使う

	float x = std::cos(pitch_rad) * std::sin(yaw_rad);
	float y = std::sin(pitch_rad);
	float z = std::cos(pitch_rad) * std::cos(yaw_rad);
	m_moveVec = { x, y, z };
	m_moveVec.Normalize();


	// 一時的
	if (InputManager::Instance().IsTriggered(VK_SHIFT))
	{
		int lv = (int)GetSpeedLevel();
		lv += 1;
		ChangeSpeedLevel((SpeedLevel)lv);
	}
	if (InputManager::Instance().IsPressed(VK_SPACE)) m_gravity -= 5.f * dt;

	m_gravity += 1.f * dt;

	

	// 位置の適用
	m_amountMove += m_moveVec * m_speed * dt;
	m_amountMove.y -= m_gravity;


	// ==========================================
	// 5. 速度の自然減衰（空気抵抗や摩擦）
	// ==========================================
	
	float friction = 0.55f; 
	m_speed *= std::exp(-friction * dt);

	// 速度が微小になったら完全に停止させる
	if (std::abs(m_speed) < 0.5f) {
		m_speed = 0.0f;

		if(m_level != SpeedLevel::Clash)ChangeSpeedLevel(SpeedLevel::Idle);
	}
}