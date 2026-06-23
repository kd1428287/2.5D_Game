#include "Player.h"

#include "Application/System/InputManager/InputManager.h"
#include "Application/System/CameraManager/CameraManager.h"
#include "Application/Scene/SceneManager.h"
#include "Application/System/Reader/Reader.h"

// =============================================================
// 初期化
// =============================================================

void Player::Init()
{
	m_pCollider = std::make_unique<KdCollider>();

	m_model = KdAssets::Instance().m_modeldatas.GetData("Asset/Models/car_van/car_van2.gltf");

	m_speed = 0.0f;
	m_angle = { 0.0f, 0.0f, 0.0f };
	m_scale = { 1.0f, 1.0f, 1.0f };
	m_level = SpeedLevel::Idle;

	ChangeSpeedLevel(SpeedLevel::Idle);
	m_acceleration = 5.0f;

	// --- イベント購読 ---

	// 衝突結果
	m_subscriber.push_back(
		GLOBALEVENT.subscribe<Events::Player::HitResult>([this](const Events::Player::HitResult& e)
			{
				if (e.type == Events::Player::HitResult::HitResultType::Ignored) return;

				int level = (int)m_level - 2;

				switch (e.type)
				{
				case Events::Player::HitResult::HitResultType::Destroyed:
					m_destroyScore += 1;
					return;

				case Events::Player::HitResult::HitResultType::Bounced:
				default:
					if (m_level != SpeedLevel::Clash && level > 0)
					{
						m_speed *= -0.5f;
						m_clashCount = (float)level * 0.1f;
						ChangeSpeedLevel(SpeedLevel::Clash);

						for (int i = 0; i < level; i++)
						{
							GLOBALEVENT.publish(Events::Else::CreateObjectEvent("Smoke", m_pos, 0.1f, false, 5.0f));
						}
					}
					else if (level <= 0)
					{
						ChangeSpeedLevel(SpeedLevel::Clash);
					}
					break;
				}
			})
	);

	// 配達完了
	m_subscriber.push_back(
		GLOBALEVENT.subscribe<Events::Player::DeliveryPointCompleted>([this](const Events::Player::DeliveryPointCompleted& e)
			{
				m_deliveryScore += 1;
				m_isDeliveryAnime = true;
				m_deliveryAnimeTime = 0.0f;
				KdDebugGUI::Instance().AddLog("%d", m_deliveryScore);

				Math::Vector3 score = Reader::Instance().ReadScore();
				if ((m_deliveryScore / 10000) + m_deliveryDestroy >= score.z)
				{
					score = { (float)m_deliveryScore, (float)m_destroyScore, score.z };
					Reader::Instance().WriteScore(score);
					GLOBALEVENT.publish(Events::Else::GameEnd());
					GLOBALEVENT.publish(Events::Else::GameToResultBegin());
				}
			})
	);

	// 配達地点破壊
	m_subscriber.push_back(
		GLOBALEVENT.subscribe<Events::Player::DeliveryPointDeleted>([this](const Events::Player::DeliveryPointDeleted& e)
			{
				m_deliveryDestroy++;
				Math::Vector3 score = Reader::Instance().ReadScore();
				if (m_deliveryScore + m_deliveryDestroy >= score.z)
				{
					score = { (float)m_deliveryScore, (float)m_destroyScore, score.z };
					Reader::Instance().WriteScore(score);
					GLOBALEVENT.publish(Events::Else::GameEnd());
					GLOBALEVENT.publish(Events::Else::GameToResultBegin());
				}
			})
	);

	// スピードアップアイテム取得
	m_subscriber.push_back(
		GLOBALEVENT.subscribe<Events::Player::GetSpeedUp>([this](const Events::Player::GetSpeedUp& e)
			{
				if (m_level >= SpeedLevel::Speed6) return;
				ChangeSpeedLevel((SpeedLevel)((int)m_level + 1));
			})
	);

	// ゲーム開始
	m_subscriber.push_back(
		GLOBALEVENT.subscribe<Events::Else::GameStart>([this](const Events::Else::GameStart& e)
			{
				m_isControllable = true;
			})
	);

	// ゲーム終了
	m_subscriber.push_back(
		GLOBALEVENT.subscribe<Events::Else::GameEnd>([this](const Events::Else::GameEnd& e)
			{
				Math::Vector3 score = Reader::Instance().ReadScore();
				score = { (float)m_deliveryScore, (float)m_destroyScore, score.z };
				Reader::Instance().WriteScore(score);
			})
	);

	m_pCollider->RegisterCollisionShape("PlayerCollision", m_model, KdCollider::Type::TypeEvent);
}

// =============================================================
// 毎フレーム更新
// =============================================================

void Player::PreUpdate()
{
	m_amountMove = Math::Vector3::Zero;
}

void Player::Update(float dt)
{
	if (!m_isControllable && !m_isAutoPilot) return;

	UpdateMove(dt);

	//if (m_level > SpeedLevel::Speed3)
	{
		/*float angle = std::atan2(m_angle.x, m_angle.z);
		GLOBALEVENT.publish(Events::Else::CreateObjectEvent("Smoke", m_pos, 0.1f, false, angle));*/
	}

	// 配達アニメーション（車体がぷるっと伸縮する）
	if (m_isDeliveryAnime)
	{
		m_deliveryAnimeTime += dt;
		float duration = (2.0f * M_PI) / m_deliveryAnimeSpeed;  // 1 サイクルの時間

		if (m_deliveryAnimeTime >= duration)
		{
			m_scale = { 1.0f, 1.0f, 1.0f };
			m_isDeliveryAnime = false;
		}
		else
		{
			float wave = sinf(m_deliveryAnimeTime * m_deliveryAnimeSpeed);
			m_scale.x = 1.0f + wave * m_deliveryAnimeAmplitude;
			m_scale.y = 1.0f - wave * m_deliveryAnimeAmplitude;
		}
	}

	// クラッシュ回復カウントダウン
	if (m_level == SpeedLevel::Clash)
	{
		m_clashCount -= dt;
		if (m_clashCount <= 0.0f)
		{
			m_clashCount = 0.0f;
			ChangeSpeedLevel(SpeedLevel::Idle);
		}
	}

	KdAudioManager::Instance().SetListnerMatrix(m_mWorld);
}

void Player::PostUpdate()
{
	Math::Matrix rotMat =
		Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_angle.y)) *
		Math::Matrix::CreateRotationX(DirectX::XMConvertToRadians(m_angle.x));

	// 近傍オブジェクトのみ当たり判定対象にする
	std::vector<std::shared_ptr<KdGameObject>> objList;
	float radius = std::abs(m_fallDistance) + 5.0f;
	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		Math::Vector3 diff = m_pos + m_amountMove - obj->GetPos();
		if (diff.Length() < radius) objList.push_back(obj);
	}

	UpdateGroundCollision(objList);
	UpdateWallCollision(objList, rotMat);

	m_pos += m_amountMove;

	m_mWorld =
		Math::Matrix::CreateScale(m_scale) *
		rotMat *
		Math::Matrix::CreateTranslation(m_pos);
}

// =============================================================
// 描画
// =============================================================

void Player::GenerateDepthMapFromLight()
{
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld);
}

void Player::DrawLit()
{
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld);
}

// =============================================================
// スピードレベル変更
// =============================================================

void Player::ChangeSpeedLevel(SpeedLevel level)
{
	if (m_level == level) return;
	m_level = level;

	switch (m_level)
	{
	case SpeedLevel::Idle:
	case SpeedLevel::Speed1: m_maxSpeed = 0.8f;  m_minSpeed = -0.4f; break;
	case SpeedLevel::Speed2: m_maxSpeed = 1.1f;  m_minSpeed = 0.8f; break;
	case SpeedLevel::Speed3: m_maxSpeed = 1.5f;  m_minSpeed = 1.1f; break;
	case SpeedLevel::Speed4: m_maxSpeed = 2.0f;  m_minSpeed = 1.5f; break;
	case SpeedLevel::Speed5: m_maxSpeed = 3.0f;  m_minSpeed = 2.0f; break;
	case SpeedLevel::Speed6: m_maxSpeed = 4.5f;  m_minSpeed = 3.0f; break;
	case SpeedLevel::Clash:  m_maxSpeed = 0.0f;  m_minSpeed = -0.8f; break;
	default: break;
	}

	m_speed = std::max(m_speed, m_minSpeed);

	GLOBALEVENT.publish(Events::Player::ChangeSpeedLevel((int)m_level));
}

// =============================================================
// 自動操縦 API
// =============================================================

void Player::StartAutoPilot(const std::vector<Math::Vector3>& waypoints)
{
	m_isAutoPilot = true;
	m_isControllable = false;   // リザルト中は手動操作を無効化
	m_waypoints = waypoints;
	m_waypointIndex = 0;
	m_autoSteerInput = 0.0f;
	m_autoAccelInput = 0.0f;

	// Idle 状態から起動する場合は最低速で動き始める
	if (m_level == SpeedLevel::Idle)
	{
		ChangeSpeedLevel(SpeedLevel::Speed1);
	}
}

void Player::StopAutoPilot()
{
	m_isAutoPilot = false;
	m_autoSteerInput = 0.0f;
	m_autoAccelInput = 0.0f;
	m_waypointIndex = 0;
	m_waypoints.clear();

	// ステアリングをセンターへ戻して完全停止
	m_speed = 0.0f;
	m_steering = 0.0f;
	ChangeSpeedLevel(SpeedLevel::Idle);
}

// =============================================================
// 自動操縦の仮想入力計算（UpdateMove から毎フレーム呼ばれる）
// =============================================================

void Player::UpdateAutoPilotInput(float dt)
{
	// ウェイポイントをすべて消化済み → 停止
	if (m_waypoints.empty() || m_waypointIndex >= (int)m_waypoints.size())
	{
		StopAutoPilot();
		return;
	}

	const Math::Vector3& target = m_waypoints[m_waypointIndex];
	Math::Vector3 toTarget = target - m_pos;
	toTarget.y = 0.0f;

	// ウェイポイント到達判定
	if (toTarget.Length() < m_waypointReachRadius)
	{
		m_waypointIndex++;
		if (m_waypointIndex >= (int)m_waypoints.size())
		{
			StopAutoPilot();
			return;
		}
	}

	// 目標方向の yaw 角を求めて m_angle.y に直接セット（ハンドル処理を介さない）
	toTarget.Normalize();
	m_angle.y = DirectX::XMConvertToDegrees(std::atan2(toTarget.x, toTarget.z));
	m_steering = 0.0f;

	// アクセル：目標レベルに達していなければ全開、達したら上限付近で絞る
	if (m_level < m_autoPilotTargetLevel)
	{
		m_autoAccelInput = 1.0f;
	}
	else
	{
		m_autoAccelInput = (m_speed < m_maxSpeed * 0.9f) ? 0.5f : 0.0f;
	}
}

// =============================================================
// 移動処理
// =============================================================

void Player::UpdateMove(float dt)
{
	// ==========================================
	// 0. 入力ソースの決定（手動 or 自動操縦）
	// ==========================================
	float steerInput = 0.0f;
	bool  accelPressed = false;
	bool  brakePressed = false;

	if (m_isAutoPilot)
	{
		UpdateAutoPilotInput(dt);
		steerInput = m_autoSteerInput;
		accelPressed = (m_autoAccelInput > 0.0f);
		brakePressed = (m_autoAccelInput < 0.0f);
	}
	else
	{
		if (InputManager::Instance().IsPressed(VK_LEFT))  steerInput = -1.0f;
		if (InputManager::Instance().IsPressed(VK_RIGHT)) steerInput = 1.0f;
		accelPressed = InputManager::Instance().IsPressed(VK_UP);
		brakePressed = InputManager::Instance().IsPressed(VK_DOWN);
	}

	// ==========================================
	// 1. ステアリングの更新（手動操作のみ）
	// ==========================================
	// 自動操縦時は m_angle.y を直接書き換えるためステアリング処理は不要
	if (!m_isAutoPilot)
	{
		if (steerInput != 0.0f)
		{
			m_steering += steerInput * m_steerSpeed * dt;
		}
		else
		{
			if (m_steering > 0.0f)
			{
				m_steering -= m_steerSpeed * dt;
				if (m_steering < 0.0f) m_steering = 0.0f;
			}
			else if (m_steering < 0.0f)
			{
				m_steering += m_steerSpeed * dt;
				if (m_steering > 0.0f) m_steering = 0.0f;
			}
		}
		m_steering = std::clamp(m_steering, -m_maxSteerAngle, m_maxSteerAngle);
	}

	// ==========================================
	// 2. アクセル / ブレーキ
	// ==========================================
	if (m_level != SpeedLevel::Clash)
	{
		if (accelPressed) m_speed += m_acceleration * dt;
		if (brakePressed) m_speed -= m_acceleration * dt * 0.5f;
	}

	// 速度がレベル下限を下回ったらレベルダウン
	if (m_speed < m_minSpeed && m_level != SpeedLevel::Idle && m_level != SpeedLevel::Clash)
	{
		ChangeSpeedLevel((SpeedLevel)((int)m_level - 1));
	}
	// Idle から加速し始めたら Speed1 へ
	if (m_level == SpeedLevel::Idle && m_speed > 0.5f)
	{
		ChangeSpeedLevel(SpeedLevel::Speed1);
	}

	m_speed = std::clamp(m_speed, m_minSpeed, m_maxSpeed);

	// ==========================================
	// 3. 車の向きの更新（バイシクルモデル）
	// ==========================================
	if (std::abs(m_speed) > 0.01f)
	{
		float steerRad = DirectX::XMConvertToRadians(m_steering);
		float angularVelocity = (m_speed * std::tan(steerRad)) / m_wheelBase;  // ω = V*tan(δ)/L
		m_angle.y += DirectX::XMConvertToDegrees(angularVelocity * dt);
	}

	// ==========================================
	// 4. 移動ベクトルの計算
	// ==========================================
	float pitch_rad = DirectX::XMConvertToRadians(m_angle.x);
	float yaw_rad = DirectX::XMConvertToRadians(m_angle.y);

	// 水平推進ベクトル（重力を混ぜない）
	Math::Vector3 forwardVec =
	{
		std::cos(pitch_rad) * std::sin(yaw_rad),
		std::sin(pitch_rad),
		std::cos(pitch_rad) * std::cos(yaw_rad)
	};
	forwardVec.Normalize();
	m_amountMove = forwardVec * m_speed * dt;

	// 重力加速
	m_fallVelocity -= GRAVITY_ACCEL * dt;
	m_fallVelocity = std::max(m_fallVelocity, MAX_FALL_SPEED);

	m_fallDistance = m_fallVelocity * dt;
	m_amountMove.y += m_fallDistance;

	// ==========================================
	// 5. 速度の自然減衰（摩擦・空気抵抗）
	// ==========================================
	float friction = (m_speed < 0.0f) ? 0.995f : 0.55f;
	m_speed *= std::exp(-friction * dt);

	if (std::abs(m_speed) < 0.01f)
	{
		m_speed = 0.0f;
		if (m_level != SpeedLevel::Clash) ChangeSpeedLevel(SpeedLevel::Idle);
	}
}

// =============================================================
// 地面との衝突処理
// =============================================================

void Player::UpdateGroundCollision(const std::vector<std::shared_ptr<KdGameObject>>& objList)
{
	KdCollider::RayInfo ray;
	ray.m_pos = m_pos;
	ray.m_dir = { 0.0f, -1.0f, 0.0f };
	ray.m_type = KdCollider::TypeGround;

	const float enableStepHigh = 0.02f;
	ray.m_pos.y += enableStepHigh;
	ray.m_range = -m_fallDistance + enableStepHigh;

	std::list<KdCollider::CollisionResult> retRayList;
	for (auto& obj : objList) obj->Intersects(ray, &retRayList);

	float         maxOverLap = 0.0f;
	Math::Vector3 hitPos;
	bool          hit = false;

	for (auto& ret : retRayList)
	{
		if (ret.m_overlapDistance > maxOverLap)
		{
			maxOverLap = ret.m_overlapDistance;
			hitPos = ret.m_hitPos;
			hit = true;
		}
	}

	if (hit)
	{
		// めり込み解消：ぴったり床の高さに合わせる
		if (m_amountMove.y + m_pos.y < hitPos.y)
		{
			m_amountMove.y = hitPos.y - m_pos.y;
		}
		m_fallVelocity = 0.0f;
		m_fallDistance = 0.0f;
	}
}

// =============================================================
// 壁との衝突処理（スフィアキャスト＋壁滑り）
// =============================================================

void Player::UpdateWallCollision(const std::vector<std::shared_ptr<KdGameObject>>& objList, const Math::Matrix& rotMat)
{
	std::set<std::shared_ptr<KdGameObject>> hitObjectsThisFrame;

	// 水平方向の推進ベクトルだけを壁滑りに使う（重力成分は別管理）
	Math::Vector3 slidingVelocity = m_amountMove;
	float savedGravityY = slidingVelocity.y;
	slidingVelocity.y = 0.0f;

	// 反復めり込み解消（L字コーナーでのスタック防止）
	for (int iteration = 0; iteration < MAX_COLLISION_ITERATIONS; ++iteration)
	{
		KdCollider::SphereInfo spheres[SPHERE_NUM];
		std::list<KdCollider::CollisionResult> retSphereList;

		for (int i = 0; i < SPHERE_NUM; ++i)
		{
			Math::Vector3 localOffset = { 0.0f, 0.0f, m_sphereOffsets[i] };
			Math::Vector3 rotatedOffset = Math::Vector3::TransformNormal(localOffset, rotMat);

			spheres[i].m_sphere.Center = m_pos + m_amountMove + m_sphereHeightOffset + rotatedOffset;
			spheres[i].m_sphere.Radius = m_sphereRadius;
			spheres[i].m_type = KdCollider::TypeBump;
		}

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

		if (retSphereList.empty()) break;

		// 法線ごとに最大めり込み量をまとめる
		struct PushInfo { Math::Vector3 dir; float maxOverlap = 0.0f; };
		std::vector<PushInfo> distinctPushes;

		for (const auto& ret : retSphereList)
		{
			Math::Vector3 dir = ret.m_hitDir;
			dir.y = 0.0f;
			if (dir.LengthSquared() <= 0.0f) continue;
			dir.Normalize();

			bool isDuplicate = false;
			for (auto& push : distinctPushes)
			{
				if (push.dir.Dot(dir) > 0.9f)  // 約 25 度以内は同一の壁とみなす
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

		for (const auto& push : distinctPushes)
		{
			// A. めり込んでいる分だけ即座に押し出す
			m_amountMove += push.dir * push.maxOverlap;

			// B. 壁滑り：推進ベクトルを壁の法線平面へ投影する
			float dotResult = slidingVelocity.Dot(push.dir);
			if (dotResult < 0.0f)
			{
				slidingVelocity -= push.dir * dotResult;
			}
		}

		m_amountMove.x = slidingVelocity.x;
		m_amountMove.z = slidingVelocity.z;
		m_amountMove.y = savedGravityY;
	}

	// 新規衝突のみイベント通知（壁こすりのスパム防止）
	for (auto& obj : hitObjectsThisFrame)
	{
		if (m_previousHitObjects.find(obj) == m_previousHitObjects.end())
		{
			KdCollider::CollisionResult dummyResult;
			GLOBALEVENT.publish(Events::Player::OnHit(shared_from_this(), obj, dummyResult));
		}
	}

	m_previousHitObjects = hitObjectsThisFrame;
}