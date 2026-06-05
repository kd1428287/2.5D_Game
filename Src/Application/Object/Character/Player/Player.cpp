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
	m_level = SpeedLevel::Speed3;

	m_acceleration = 0.05f;
	m_acceleration = 1.0f;

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
	
	ActiveInput();
	UpdateMove(dt);
	

	/*m_mWorld = 
		Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_angle.y)) *
		Math::Matrix::CreateRotationX(DirectX::XMConvertToRadians(m_angle.x)) *
		Math::Matrix::CreateTranslation(m_pos);*/
}

void Player::PostUpdate()
{
	// 当たり判定はココ！！
		// レイ判定 (ゲロ重い)

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

	// 全オブジェクトと当たり判定
	for (auto& obj : SceneManager::Instance().GetObjList())
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
	Math::Matrix rotMat = 
		Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_angle.y)) *
		Math::Matrix::CreateRotationX(DirectX::XMConvertToRadians(m_angle.x));

	const int SPHERE_NUM = 3;
	float zOffset[SPHERE_NUM] = { 0.7f,0.0f,-0.7f };

	KdCollider::SphereInfo sphere[SPHERE_NUM];

	for (int i = 0; i < SPHERE_NUM; ++i)
	{
		
		// ローカルのズレ(Z方向)を、車の現在の向き(回転行列)に合わせてワールド空間のズレに変換
		Math::Vector3 offset = { 0.0f, 0.0f, zOffset[i] };
		Math::Vector3 rotatedOffset = Math::Vector3::TransformNormal(offset, rotMat);

		// 球の中心座標 ＝ 車の中心座標 ＋ 高さ調整 ＋ 向きを考慮した前後のズレ
		sphere[i].m_sphere.Center = m_pos + m_amountMove + Math::Vector3(0.0f, 0.6f, 0.0f) + rotatedOffset;

		// 車幅の半分くらいを半径に設定
		sphere[i].m_sphere.Radius = 0.5f;
		sphere[i].m_type = KdCollider::TypeGround; 

		// デバッグ描画（赤いワイヤーフレームで球を描画）
		m_pDebugWire->AddDebugSphere(sphere[i].m_sphere.Center, sphere[i].m_sphere.Radius, { 1.0f, 0.0f, 0.0f, 1.0f });
	}

	std::list<KdCollider::CollisionResult> retSphereList;

	// 全オブジェクトと当たり判定
	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		for (int i = 0; i < SPHERE_NUM; i++)
		{
			if (obj->Intersects(sphere[i], &retSphereList))break;
		}
	}

	maxOverLap = 0.0f;
	hit = false;
	Math::Vector3 hitDir;

	for (auto& ret : retSphereList)
	{
		if (maxOverLap < ret.m_overlapDistance)
		{
			maxOverLap = ret.m_overlapDistance;
			hitDir = ret.m_hitDir;
			hit = true;
		}
	}

	if (hit)
	{
		// ベクトルを再正規化（長さを1にする）
		hitDir.Normalize();

		// 押し出しを実行してプレイヤーの座標を更新
		m_amountMove += hitDir * maxOverLap;
	}

	// ----------------------------------------------------
	// 全ての押し出しが終わった後の最終的な m_pos で m_mWorld を確定させる
	// ----------------------------------------------------

	m_pos += m_amountMove;

	m_mWorld = 
		//Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_angle.y)) *
		//Math::Matrix::CreateRotationX(DirectX::XMConvertToRadians(m_angle.x)) *
		rotMat * 
		Math::Matrix::CreateTranslation(m_pos);

	KdDebugGUI::Instance().AddLog("X:%f,\nY:%f,\nZ:%f,\n",m_pos.x,m_pos.y,m_pos.z);
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
	case SpeedLevel::Speed1: m_maxSpeed = 0.2f; break;
	case SpeedLevel::Speed2: m_maxSpeed = 0.3f; break;
	case SpeedLevel::Speed3: m_maxSpeed = 0.4f; break;
	case SpeedLevel::Speed4: m_maxSpeed = 0.5f; break;
	case SpeedLevel::Speed5: m_maxSpeed = 0.7f; break;
	case SpeedLevel::Clash:  m_maxSpeed = 0.0f; break;
	default: break;
	}
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
	if (InputManager::Instance().IsPressed(VK_UP))
	{
		m_speed += m_acceleration * dt;
	}
	if (InputManager::Instance().IsPressed(VK_DOWN))
	{
		// ブレーキ（あるいはバック）
		m_speed -= m_acceleration * dt * 0.2;
	}


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


	// 一時的な上下移動（既存のまま）
	if (InputManager::Instance().IsPressed(VK_SHIFT)) m_gravity += 0.1f * dt;
	if (InputManager::Instance().IsPressed(VK_SPACE)) m_gravity -= 0.1f * dt;

	m_gravity += 0.01f * dt;

	// 位置の適用
	m_amountMove += m_moveVec * m_speed * dt;
	m_amountMove.y -= m_gravity;


	// ==========================================
	// 5. 速度の自然減衰（空気抵抗や摩擦）
	// ==========================================
	// ※ m_speed *= 0.85f * dt; だとフレームレート依存で急ブレーキがかかるため修正
	float friction = 0.15f; 
	m_speed -= m_speed * friction * dt;// *60.0f;

	// 速度が微小になったら完全に停止させる
	if (m_speed > -0.005f && m_speed < 0.005f) {
		m_speed = 0.0f;
	}
}