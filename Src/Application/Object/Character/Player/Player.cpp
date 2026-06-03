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
	m_model->Load("Asset/Models/car_van/car_van.gltf");

	//カメラ追従対象設定
	CameraManager::Instance().SetCameraTarget(this);
	
	m_pos = { 0,5,0 };
	m_speed = 0.0f;
	m_angle = { 0,0,0 };

	m_acceleration = 0.1f;
	m_acceleration = 1.0f;
	m_turningForce = 10.0f;
}

void Player::PreUpdate()
{
	
}

void Player::Update(float dt)
{
	ActiveInput();
	UpdateMove(dt);
	

	m_mWorld = 
		Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_angle.z)) *
		Math::Matrix::CreateRotationX(DirectX::XMConvertToRadians(m_angle.x)) *
		Math::Matrix::CreateTranslation(m_pos);
}

void Player::PostUpdate()
{
	// 当たり判定はココ！！
		// レイ判定 (ゲロ重い)

		// 当たる側の処理
		// レイ判定用の変数を設定
	KdCollider::RayInfo ray;
	// レイの発射位置を設定
	ray.m_pos = m_pos;
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
		m_pos = hitPos +Math::Vector3(0, -0.1f, 0);
		m_gravity = 0.0f;
	}

	
	// 球(スフィア)判定=========
	Math::Matrix rotMat = 
		Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_angle.z)) *
		Math::Matrix::CreateRotationX(DirectX::XMConvertToRadians(m_angle.x));

	const int SPHERE_NUM = 3;
	float zOffset[SPHERE_NUM] = { 0.7f,0.0f,-0.7f };

	for (int i = 0; i < SPHERE_NUM; ++i)
	{
		KdCollider::SphereInfo sphere;

		// ローカルのズレ(Z方向)を、車の現在の向き(回転行列)に合わせてワールド空間のズレに変換
		Math::Vector3 offset = { 0.0f, 0.0f, zOffset[i] };
		Math::Vector3 rotatedOffset = Math::Vector3::TransformNormal(offset, rotMat);

		// 球の中心座標 ＝ 車の中心座標 ＋ 高さ調整 ＋ 向きを考慮した前後のズレ
		sphere.m_sphere.Center = m_pos + Math::Vector3(0.0f, 0.5f, 0.0f) + rotatedOffset;

		// 車幅の半分くらいを半径に設定
		sphere.m_sphere.Radius = 0.5f;
		sphere.m_type = KdCollider::TypeBump; 

		// デバッグ描画（赤いワイヤーフレームで球を描画）
		m_pDebugWire->AddDebugSphere(sphere.m_sphere.Center, sphere.m_sphere.Radius, { 1.0f, 0.0f, 0.0f, 1.0f });

		std::list<KdCollider::CollisionResult> retSphereList;

		// 全オブジェクトと当たり判定
		for (auto& obj : SceneManager::Instance().GetObjList())
		{
			obj->Intersects(sphere, &retSphereList);
		}

		float maxOverLap = 0.0f;
		bool hit = false;
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
			// 床面の斜めポリゴンなどで上に押し出されるのを防ぐため、Y方向のベクトルを消す
			hitDir.y = 0.0f;

			// ベクトルを再正規化（長さを1にする）
			hitDir.Normalize();

			// 押し出しを実行してプレイヤーの座標を更新
			m_pos += hitDir * maxOverLap;
		}
	}

	// ----------------------------------------------------
	// 全ての押し出しが終わった後の最終的な m_pos で m_mWorld を確定させる
	// ----------------------------------------------------
	m_mWorld = 
		Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_angle.z)) *
		Math::Matrix::CreateRotationX(DirectX::XMConvertToRadians(m_angle.x)) *
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
{}

void Player::ActiveInput()
{
	/*if (InputManager::Instance().IsTriggered(VK_LBUTTON))
	{
		m_state = PlayerState::Attack_start;
	}*/

	
}

void Player::UpdateMove(float dt)
{
	float zRotation = 0.0f;
	if (InputManager::Instance().IsPressed(VK_UP))
	{
		m_speed += m_acceleration * dt;
	}
	if (InputManager::Instance().IsPressed(VK_DOWN))
	{
		m_speed -= 0.02f;
	}
	if (InputManager::Instance().IsPressed(VK_LEFT))
	{
		zRotation += -m_turningForce * dt;
	}
	if (InputManager::Instance().IsPressed(VK_RIGHT))
	{
		zRotation += m_turningForce * dt;
	}

	// 一時的な上下移動
	if (InputManager::Instance().IsPressed(VK_SHIFT))
	{
		m_gravity += 0.1f * dt;
		//m_pos.y -= 0.05f;
	}
	if (InputManager::Instance().IsPressed(VK_SPACE))
	{
		m_gravity -= 0.1f * dt;
	}

	float pitch_rad = DirectX::XMConvertToRadians(m_angle.x);
	float yaw_rad = DirectX::XMConvertToRadians(m_angle.z + zRotation);
	float x = std::cos(pitch_rad) * std::sin(yaw_rad);
	float y = std::sin(pitch_rad);
	float z = std::cos(pitch_rad) * std::cos(yaw_rad);
	m_moveVec = { x, y, z };

	float camAng = 0;
	if (zRotation > 0)camAng = -10.0f;
	else if (zRotation < 0)camAng = 10.0f;

	//CameraManager::Instance().SetCameraAngleY((zRotation * -1.0f));
	CameraManager::Instance().SetCameraAngleY(camAng);

	// 速度レベル毎の上限処理
	switch (m_level)
	{
	case SpeedLevel::Idle:
	case SpeedLevel::Speed1:
		if (m_speed > 0.2f)m_speed = 0.2f;
		break;
	case SpeedLevel::Speed2:
		break;
	case SpeedLevel::Speed3:
		break;
	case SpeedLevel::Speed4:
		break;
	case SpeedLevel::Speed5:
		break;
	case SpeedLevel::Clash:
		break;
	default:
		break;
	}

	m_gravity += 0.01f * dt;

	m_moveVec.Normalize();
	m_pos += m_moveVec * m_speed * dt;
	m_pos.y -= m_gravity;
	m_speed *= 0.85f * dt ;
	if (m_speed > -0.005f && m_speed < 0.005f)m_speed = 0.0f;
	else m_angle.z += zRotation * m_speed * dt;
}
