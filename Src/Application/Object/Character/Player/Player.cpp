#include "Player.h"

#include "Application/System/InputManager/InputManager.h"
#include "Application/System/CameraManager/CameraManager.h"
#include "Application/Scene/SceneManager.h"
#include "Application/System/Reader/Reader.h"

void Player::Init()
{
	// 当たり判定
	m_pCollider = std::make_unique<KdCollider>();

	// モデル
	m_model = std::make_shared<KdModelData>();
	m_model = KdAssets::Instance().m_modeldatas.GetData("Asset/Models/car_van/car_van2.gltf");
	
	m_speed = 0.0f;
	m_angle = { 0,0,0 };
	m_level = SpeedLevel::Idle;

	ChangeSpeedLevel(SpeedLevel::Idle);

	m_acceleration = 5.0f;
	m_scale = { 1,1,1 };

	m_subscriber.push_back(
		GLOBALEVENT.subscribe<Events::Player::HitResult>([this](const Events::Player::HitResult& e)
		{
			if (e.type == Events::Player::HitResult::HitResultType::Ignored)return;

			int level = (int)m_level - 2;
		
			switch (e.type)
			{
			case Events::Player::HitResult::HitResultType::Destroyed:
				m_destroyScore += 1000;
				return;
			case Events::Player::HitResult::HitResultType::Bounced:
			default:
				if (m_level != SpeedLevel::Clash && level > 0)
				{
					m_speed *= -0.5f;
					m_clashCount = (float)level * 0.1f;
					ChangeSpeedLevel(SpeedLevel::Clash);

					for (int i = 0; i < (level); i++)
					{
						GLOBALEVENT.publish(Events::Else::CreateObjectEvent("Smoke", m_pos, 0.1f, false, 5.f));
					}
				}
				else if (level <= 0)
				{
					ChangeSpeedLevel(SpeedLevel::Idle);
				}
				break;
			}
		})
	);

	m_subscriber.push_back(
		GLOBALEVENT.subscribe<Events::Player::DeliveryPointCompleted>([this](const Events::Player::DeliveryPointCompleted& e)
			{
				m_deliveryScore += 10000;
				m_isDeliveryAnime = true;
				m_deliveryAnimeTime = 0.f;
				KdDebugGUI::Instance().AddLog("%d", m_deliveryScore);

				Math::Vector3 score = Reader::Instance().ReadScore();
				if ((m_deliveryScore / 10000) + m_deliveryDestroy >= score.z)
				{
					score = { (float)m_deliveryScore, (float)m_destroyScore,score.z };
					Reader::Instance().WriteScore(score);
					GLOBALEVENT.publish(Events::Else::GameEnd());
					GLOBALEVENT.publish(Events::Else::GameToResultBegin());
				}
			})
	);

	m_subscriber.push_back(
		GLOBALEVENT.subscribe<Events::Player::DeliveryPointDeleted>([this](const Events::Player::DeliveryPointDeleted& e)
			{
				m_deliveryDestroy++;
				Math::Vector3 score = Reader::Instance().ReadScore();
				if ((m_deliveryScore / 10000) + m_deliveryDestroy >= score.z)
				{
					score = { (float)m_deliveryScore, (float)m_destroyScore,score.z };
					Reader::Instance().WriteScore(score);
					GLOBALEVENT.publish(Events::Else::GameEnd());
					GLOBALEVENT.publish(Events::Else::GameToResultBegin());
				}
			})
	);

	m_subscriber.push_back(
		GLOBALEVENT.subscribe<Events::Player::GetSpeedUp>([this](const Events::Player::GetSpeedUp& e)
			{
				if (m_level >= SpeedLevel::Speed6)return;
				int level = (int)m_level + 1;
				ChangeSpeedLevel((SpeedLevel)level);
			})
	);

	m_subscriber.push_back(
		GLOBALEVENT.subscribe<Events::Else::GameStart>([this](const Events::Else::GameStart& e)
			{
				m_isControllable = true;
			})
	);

	m_subscriber.push_back(
		GLOBALEVENT.subscribe<Events::Else::GameEnd>([this](const Events::Else::GameEnd& e)
			{
				Math::Vector3 score = Reader::Instance().ReadScore();
				score = { (float)m_deliveryScore, (float)m_destroyScore,score.z };
				Reader::Instance().WriteScore(score);
			})
	);

	m_pCollider->RegisterCollisionShape(
		"PlayerCollision",
		m_model,
		KdCollider::Type::TypeEvent 
	);
}

void Player::PreUpdate()
{
	m_amountMove = Math::Vector3::Zero;
}

void Player::Update(float dt)
{
	if (!m_isControllable)return;

	UpdateMove(dt);

	// 正規化した位相で制御
	if(m_isDeliveryAnime)
	{
		m_deliveryAnimeTime += dt;
		float duration = (2.0f * M_PI) / m_deliveryAnimeSpeed; // 1フルサイクルの時間

		if (m_deliveryAnimeTime >= duration)
		{
			m_scale = { 1, 1, 1 };
			m_isDeliveryAnime = false;
		}
		else
		{
			// sin(0→2π) で 0→1→0→-1→0 の1周
			float wave = sinf(m_deliveryAnimeTime * m_deliveryAnimeSpeed);
			m_scale.x = 1.0f + wave * m_deliveryAnimeAmplitude;
			m_scale.y = 1.0f - wave * m_deliveryAnimeAmplitude;
		}
	}

	

	if (m_level == SpeedLevel::Clash)
	{
		m_clashCount -= dt;
		if (m_clashCount <= 0.0f)
		{
			ChangeSpeedLevel(SpeedLevel::Idle); 
			m_clashCount = 0.0f;
		}
	}

	KdAudioManager::Instance().SetListnerMatrix(m_mWorld);
}

void Player::PostUpdate()
{
	Math::Matrix rotMat =
		Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_angle.y)) *
		Math::Matrix::CreateRotationX(DirectX::XMConvertToRadians(m_angle.x));

	// 全オブジェクトの中から自機から一定距離内のオブジェクトを取得
	std::vector<std::shared_ptr<KdGameObject>> objList;
	Math::Vector3 length;
	float radius = std::abs(m_fallDistance) + 5.0f;
	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		length = m_pos + m_amountMove - obj->GetPos();
		if (length.Length() < radius)objList.push_back(obj);
	}

	// 当たる側の処理
	UpdateGroundCollision(objList);

	// 球(スフィア)判定=========
	UpdateWallCollision(objList, rotMat);
	
	// ----------------------------------------------------
	// 全ての押し出しが終わった後の最終的な m_pos で m_mWorld を確定させる
	// ----------------------------------------------------

	m_pos += m_amountMove;

	m_mWorld =
		Math::Matrix::CreateScale(m_scale) * 
		rotMat *
		Math::Matrix::CreateTranslation(m_pos);

	if (m_level > SpeedLevel::Speed3)
	{
		//GLOBALEVENT.publish(Events::Else::CreateObjectEvent("Smoke", m_pos, 0.1f, true, 10.f));
	}
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
		m_maxSpeed = 0.8f;
		m_minSpeed = -0.4f;
		break;
	case SpeedLevel::Speed2: 
		m_maxSpeed = 1.1f; 
		m_minSpeed = 0.8f;
		break;
	case SpeedLevel::Speed3: 
		m_maxSpeed = 1.5f; 
		m_minSpeed = 1.1f;
		break;
	case SpeedLevel::Speed4: 
		m_maxSpeed = 2.f; 
		m_minSpeed = 1.5f;
		break;
	case SpeedLevel::Speed5: 
		m_maxSpeed = 3.f; 
		m_minSpeed = 2.0f;
		break;
	case SpeedLevel::Speed6:
		m_maxSpeed = 4.5f;
		m_minSpeed = 3.0f;
		break;
	case SpeedLevel::Clash:  
		m_maxSpeed = 0.f;
		m_minSpeed = -0.8f;
		break;
	default: break;
	}

	m_speed = std::max(m_speed, m_minSpeed);

	GLOBALEVENT.publish(Events::Player::ChangeSpeedLevel((int)m_level));
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
			m_speed -= m_acceleration * dt * 0.5f;
		}
	}


	if (m_speed < m_minSpeed && m_level != SpeedLevel::Idle && m_level != SpeedLevel::Clash)ChangeSpeedLevel((SpeedLevel)((int)m_level - 1));
	if (m_level == SpeedLevel::Idle && m_speed > std::abs(0.5f))ChangeSpeedLevel(SpeedLevel::Speed1);

	// 速度レベル毎の上限処理（既存のまま）
	if (m_speed > m_maxSpeed)m_speed = m_maxSpeed;
	if (m_speed < m_minSpeed)m_speed = m_minSpeed;

	// 一時的
	if (InputManager::Instance().IsTriggered(VK_SHIFT))
	{
		int lv = (int)GetSpeedLevel();
		lv += 1;
		ChangeSpeedLevel((SpeedLevel)lv);
	}
	


	// ==========================================
	// 3. 車の向き（ヘディング）の更新 (バイシクルモデル)
	// ==========================================
	// 車が動いている時だけ向きが変わるようにする
	if (std::abs(m_speed) > 0.01f)
	{
		// C++の標準数学関数はラジアンを要求するため変換
		float steerRad = DirectX::XMConvertToRadians(m_steering);

		// 角速度 ω = (V * tan(δ)) / L
		float angularVelocity = (m_speed * std::tan(steerRad)) / m_wheelBase;

		m_angle.y += DirectX::XMConvertToDegrees(angularVelocity * dt);
	}


	// ==========================================
// 4. 移動ベクトルの計算と位置の更新 (修正版)
// ==========================================
	float pitch_rad = DirectX::XMConvertToRadians(m_angle.x);
	float yaw_rad = DirectX::XMConvertToRadians(m_angle.y);

	// ① 水平方向（車の推進力）の計算
	// ※ここでは重力を絶対に混ぜない。純粋な車の向いている方向だけを抽出。
	Math::Vector3 forwardVec = {
		std::cos(pitch_rad) * std::sin(yaw_rad),
		std::sin(pitch_rad),
		std::cos(pitch_rad) * std::cos(yaw_rad)
	};
	forwardVec.Normalize(); // 推進方向だけを正規化

	// 前進移動量を確定
	m_amountMove = forwardVec * m_speed * dt;

	// ② 垂直方向（重力による落下）の計算
	// ジャンプの処理（元コードの VK_SPACE に該当）
	if (InputManager::Instance().IsTriggered(VK_SPACE)) {
		m_fallVelocity = 5.0f; // 上向きの初速を与える
	}

	// 毎フレーム、重力加速度を落下速度に加算 (v = v0 - at)
	m_fallVelocity -= GRAVITY_ACCEL * dt;

	// 落下速度が無限に大きくならないようにクランプ（終端速度）
	m_fallVelocity = std::max(m_fallVelocity, MAX_FALL_SPEED);

	// ③ Y軸の移動量に落下速度を適用
	m_fallDistance = m_fallVelocity* dt;
	m_amountMove.y += m_fallDistance;


	// ==========================================
	// 5. 速度の自然減衰（空気抵抗や摩擦）
	// ==========================================
	
	float friction = 0.55f; 
	if (m_speed < 0)friction = 0.995f;
	m_speed *= std::exp(-friction * dt);

	// 速度が微小になったら完全に停止させる
	if (std::abs(m_speed) < 0.01f) {
		m_speed = 0.0f;

		if(m_level != SpeedLevel::Clash)ChangeSpeedLevel(SpeedLevel::Idle);
	}
}

void Player::UpdateGroundCollision(const std::vector<std::shared_ptr<KdGameObject>>& objList)
{
	// レイ判定用の変数を設定
	KdCollider::RayInfo ray;
	// レイの発射位置を設定
	ray.m_pos = m_pos;
	// 段差の許容範囲を設定
	float enableStepHigh = 0.02f;
	ray.m_pos.y += enableStepHigh;
	// レイの発射方向を設定
	ray.m_dir = { 0,-1,0 };
	// レイの長さを設定
	ray.m_range = -m_fallDistance + enableStepHigh;
	// 当たり判定を行いたいタイプを設定
	ray.m_type = KdCollider::TypeGround;
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
		// 床に当たっていたらめり込みを解消
		if (m_amountMove.y + m_pos.y < hitPos.y)
		{
			m_amountMove.y = hitPos.y - m_pos.y; // ぴったり床の高さに合わせる
		}

		// 床に接地している間は落下速度をリセット（または微小なマイナス値にして接地判定を維持する）
		m_fallVelocity = 0.0f;
		m_fallDistance = 0.0f;
	}
}

void Player::UpdateWallCollision(const std::vector<std::shared_ptr<KdGameObject>>& objList, const Math::Matrix& rotMat)
{
	// 同一フレーム内で同じオブジェクトへの衝突イベントが重複するのを防ぐ
	std::set<std::shared_ptr<KdGameObject>> hitObjectsThisFrame;

	// 1. 純粋な「車の推進ベクトル」を抽出（重力などの垂直成分を除外）
	// これを行うことで、壁に阻まれても「バックする力」や「旋回する力」が殺されなくなります。
	Math::Vector3 slidingVelocity = m_amountMove;
	float savedGravityY = slidingVelocity.y; // 重力を一時退避
	slidingVelocity.y = 0.0f;                // 水平方向の移動のみ計算対象にする

	bool hasCollided = false;

	// 2. 反復めり込み解消ループ（これがL字コーナーでのスタックを完全に粉砕します）
	for (int iteration = 0; iteration < MAX_COLLISION_ITERATIONS; ++iteration)
	{
		KdCollider::SphereInfo spheres[SPHERE_NUM];
		std::list<KdCollider::CollisionResult> retSphereList;

		// 現在の計算途中の m_amountMove を適用した仮の位置にスフィアを配置
		for (int i = 0; i < SPHERE_NUM; ++i)
		{
			Math::Vector3 localOffset = { 0.0f, 0.0f, m_sphereOffsets[i] };
			Math::Vector3 rotatedOffset = Math::Vector3::TransformNormal(localOffset, rotMat);

			spheres[i].m_sphere.Center = m_pos + m_amountMove + m_sphereHeightOffset + rotatedOffset;
			spheres[i].m_sphere.Radius = m_sphereRadius;
			spheres[i].m_type = KdCollider::TypeBump;
		}

		// 衝突検知
		for (auto& obj : objList)
		{
			for (int i = 0; i < SPHERE_NUM; ++i)
			{
				if (obj->Intersects(spheres[i], &retSphereList))
				{
					hitObjectsThisFrame.insert(obj);
				}
			}
		}

		// めり込みが完全になくなったら、これ以上位置を修正する必要がないので即ループ脱出（最適化）
		if (retSphereList.empty()) break;

		hasCollided = true;

		// 法線（壁の向き）ごとに最大のめり込み量をまとめる
		struct PushInfo { Math::Vector3 dir; float maxOverlap = 0.0f; };
		std::vector<PushInfo> distinctPushes;

		for (const auto& ret : retSphereList)
		{
			Math::Vector3 dir = ret.m_hitDir;
			dir.y = 0.0f; // 完全な横壁判定のため垂直反発はカット
			if (dir.LengthSquared() <= 0.0f) continue;
			dir.Normalize();
				
			bool isDuplicate = false;
			for (auto& push : distinctPushes)
			{
				if (push.dir.Dot(dir) > 0.9f) // 約25度以内の壁は同一の壁とみなす
				{
					isDuplicate = true;
					if (ret.m_overlapDistance > push.maxOverlap)
					{
						push.maxOverlap = ret.m_overlapDistance;
					}
					break;
				}
			}
			if (!isDuplicate)
			{
				distinctPushes.push_back({ dir, ret.m_overlapDistance });
			}
		}

		// 3. 位置の押し出しと、推進ベクトルの正確な投影（壁滑り）
		for (const auto& push : distinctPushes)
		{
			// A. 位置の強制排除（めり込んでいる分だけ即座に押し出す）
			m_amountMove += push.dir * push.maxOverlap;

			// B. 正確な壁滑り
			// 車が進もうとするベクトル（slidingVelocity）が壁に向いている（内積がマイナス）場合のみ、
			// 壁の法線平面へベクトルを投影し、壁に沿って滑らせる
			float dotResult = slidingVelocity.Dot(push.dir);
			if (dotResult < 0.0f)
			{
				slidingVelocity -= push.dir * dotResult;
			}
		}

		// 滑り処理が適用された推進力を全体の移動量に書き戻す
		m_amountMove.x = slidingVelocity.x;
		m_amountMove.z = slidingVelocity.z;
		m_amountMove.y = savedGravityY; // 退避していた重力を維持
	}

	// 5. 重複のない安全なイベント通知
	for (auto& obj : hitObjectsThisFrame)
	{
		// 前のフレームで当たっていない「新規の衝突」の場合のみ OnHit を発火する
		// これにより、角に挟まって壁にこすり続けている間はスパム通知が飛ばなくなる
		if (m_previousHitObjects.find(obj) == m_previousHitObjects.end())
		{
			KdCollider::CollisionResult dummyResult;
			GLOBALEVENT.publish(Events::Player::OnHit(shared_from_this(), obj, dummyResult));
		}
	}

	// 現在のフレームで接触しているオブジェクトのリストを、次のフレームの比較用に保存
	m_previousHitObjects = hitObjectsThisFrame;
}