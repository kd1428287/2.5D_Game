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

	m_moveVec = Math::Vector3::Zero;
	m_velocity = Math::Vector3::Zero;  // 追加
	m_speed = 0.0f;
	m_angle = { 0.0f, 0.0f, 0.0f };
	m_scale = { 1.0f, 1.0f, 1.0f };
	m_level = SpeedLevel::Idle;
	m_isDrifting = false;  // 追加
	m_wasDrifting = false;  // 追加
	m_driftDir = 0.0f;  // 追加
	m_driftAngle = 0.0f;  // 追加

	ChangeSpeedLevel(SpeedLevel::Idle);
	m_acceleration = 5.0f;

	// --- イベント購読 ---

	m_subscriber.push_back(
		GLOBALEVENT.subscribe<Events::Player::HitResult>([this](const Events::Player::HitResult& e)
			{
				if (e.type == Events::Player::HitResult::HitResultType::Ignored) return;

				int level = (int)m_level - 2;

				switch (e.type)
				{
				case Events::Player::HitResult::HitResultType::Destroyed:
					m_destroyScore += 1;
					if (m_isDrifting)
					{
						level = (int)m_level - 1;
						ChangeSpeedLevel((SpeedLevel)level);
					}
					return;

				case Events::Player::HitResult::HitResultType::Bounced:
				default:
					if (m_level != SpeedLevel::Clash && level > 0)
					{
						m_speed *= -0.5f;
						m_velocity = Math::Vector3::Zero;  // 追加: クラッシュ時に速度ベクトルもリセット
						m_isDrifting = false;                // 追加: ドリフト強制解除
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

	m_subscriber.push_back(
		GLOBALEVENT.subscribe<Events::Player::GetSpeedUp>([this](const Events::Player::GetSpeedUp& e)
			{
				if (m_level >= SpeedLevel::Speed6) return;
				ChangeSpeedLevel((SpeedLevel)((int)m_level + 1));
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
	if (!m_isControllable) return;

	UpdateMove(dt);

	// 配達アニメーション
	if (m_isDeliveryAnime)
	{
		m_deliveryAnimeTime += dt;
		float duration = (2.0f * M_PI) / m_deliveryAnimeSpeed;

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

	Events::Else::CreateObjectEvent::ObjectParameter param;
	param.m_pos = GetPos();
	param.m_vector = m_moveVec;
	param.m_float1 = std::atan2(m_moveVec.z, m_moveVec.x);
	GLOBALEVENT.publish(Events::Else::CreateObjectEvent("CarDust",param));

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
// 移動処理
// =============================================================

void Player::UpdateMove(float dt)
{
	float steerInput = 0.0f;
	bool  accelPressed = false;
	bool  backPressed = false;
	bool  brakePressed = false;
	
	if (InputManager::Instance().IsPressed(VK_LEFT))  steerInput = -1.0f;
	if (InputManager::Instance().IsPressed(VK_RIGHT)) steerInput = 1.0f;

	brakePressed = InputManager::Instance().IsPressed(VK_SPACE);
	if (!brakePressed)
	{
		accelPressed = InputManager::Instance().IsPressed(VK_UP);
		backPressed = InputManager::Instance().IsPressed(VK_DOWN);
	}

	// ==========================================
	// 1. ドリフト判定
	// ==========================================
	// クレイジータクシー式: アクセル+ブレーキ+ステアの同時押しで発動
	// ブレーキだけでは減速、アクセルも踏むことで滑らせながら加速できる

	//bool canDrift = (m_level >= SpeedLevel::Speed3)
	//	//&& accelPressed
	//	&& brakePressed
	//	&& (std::abs(steerInput) > 0.0f)
	//	&& (m_speed >= k_driftTriggerSpeed);

	bool canDrift = brakePressed
		&& (std::abs(steerInput) > 0.0f)
		&& (m_speed >= k_driftTriggerSpeed);

	if (canDrift && !m_isDrifting)
	{
		m_isDrifting = true;
		m_driftDir = steerInput;   // 開始時のステア方向を記録
		m_driftStartAngleRad = DirectX::XMConvertToRadians(m_angle.y);
		GLOBALEVENT.publish(Events::Player::DriftResult(Events::Player::DriftResult::DriftResultType::Begin));
	}

	// 入力が途切れたら解除
	if (m_isDrifting && !canDrift)
		m_isDrifting = false;

	// ==========================================
	// 2. アクセル / ブレーキ
	// ==========================================

	if (m_level != SpeedLevel::Clash)
	{
		if (m_isDrifting)
		{
			// ドリフト中: アクセルで緩やかに加速(ブレーキも踏んでいるが推進力が勝る)
			m_speed += m_acceleration * k_driftAccelScale * dt;
		}
		else
		{
			if (accelPressed) m_speed += m_acceleration * dt;
			if (backPressed) m_speed -= m_acceleration * dt * 0.5f;
		}
	}

	if (m_speed < m_minSpeed && m_level != SpeedLevel::Idle && m_level != SpeedLevel::Clash)
		ChangeSpeedLevel((SpeedLevel)((int)m_level - 1));

	if (m_level == SpeedLevel::Idle && m_speed > 0.5f)
		ChangeSpeedLevel(SpeedLevel::Speed1);

	if (InputManager::Instance().IsTriggered(VK_SHIFT))
		ChangeSpeedLevel(SpeedLevel(int(m_level) + 1));

	m_speed = std::clamp(m_speed, m_minSpeed, m_maxSpeed);

	// ==========================================
	// 3. 車体の向き更新
	// ==========================================

	if (m_isDrifting)
	{
		// ── ドリフト中の旋回 ──────────────────────
		// 通常より旋回レートを上げつつ、開始角からの累積回転を上限でクランプ
		float driftSteerRate = m_steerSpeed * k_driftSteerScale;
		m_steering += m_driftDir * driftSteerRate * dt;
		m_steering = std::clamp(m_steering, -m_maxSteerAngle, m_maxSteerAngle);

		float steerRad = DirectX::XMConvertToRadians(m_steering);
		float angularVelocity = (m_speed * std::tan(steerRad)) / m_wheelBase;
		m_angle.y += DirectX::XMConvertToDegrees(angularVelocity * dt);

		// 開始角からの累積回転を上限クランプ
		float currentRad = DirectX::XMConvertToRadians(m_angle.y);
		float rotated = currentRad - m_driftStartAngleRad;
		if (rotated > DirectX::XM_PI) rotated -= DirectX::XM_2PI;
		if (rotated < -DirectX::XM_PI) rotated += DirectX::XM_2PI;

		float maxRot = DirectX::XMConvertToRadians(k_driftMaxRotationDeg);
		if (m_driftDir > 0.0f && rotated > maxRot)
			m_angle.y = DirectX::XMConvertToDegrees(m_driftStartAngleRad + maxRot);
		else if (m_driftDir < 0.0f && rotated < -maxRot)
			m_angle.y = DirectX::XMConvertToDegrees(m_driftStartAngleRad - maxRot);
	}
	else
	{
		// ── 通常ステアリング ──────────────────────
		if (steerInput != 0.0f)
		{
			m_steering += steerInput * m_steerSpeed * dt;
		}
		else
		{
			if (m_steering > 0.0f) { m_steering -= m_steerSpeed * dt; if (m_steering < 0.0f) m_steering = 0.0f; }
			else if (m_steering < 0.0f) { m_steering += m_steerSpeed * dt; if (m_steering > 0.0f) m_steering = 0.0f; }
		}
		m_steering = std::clamp(m_steering, -m_maxSteerAngle, m_maxSteerAngle);

		if (std::abs(m_speed) > 0.01f)
		{
			
			float steerRad = DirectX::XMConvertToRadians(m_steering);
			float angularVelocity = 0.f;
			if (m_level == SpeedLevel::Speed1)
			{
				angularVelocity = (m_speed * std::tan(steerRad)) / m_wheelBase;
			}
			else {
				angularVelocity = (std::tan(steerRad)) / m_wheelBase;
			}
			//float angularVelocity = (level * std::tan(steerRad)) / m_wheelBase;
			m_angle.y += DirectX::XMConvertToDegrees(angularVelocity * dt);
		}
	}

	// ==========================================
	// 4. 車体前方ベクトル
	// ==========================================

	float pitch_rad = DirectX::XMConvertToRadians(m_angle.x);
	float yaw_rad = DirectX::XMConvertToRadians(m_angle.y);

	m_moveVec =
	{
		std::cos(pitch_rad) * std::sin(yaw_rad),
		std::sin(pitch_rad),
		std::cos(pitch_rad) * std::cos(yaw_rad)
	};
	m_moveVec.Normalize();

	// ==========================================
	// 5. 速度ベクトル合成
	// ==========================================

	if (!m_isDrifting && m_wasDrifting)
	{
		// ── ドリフト解除: スリングショット ────────
		// 車体向きへスナップ + 瞬間ブースト
		float speed2d = std::sqrt(m_velocity.x * m_velocity.x + m_velocity.z * m_velocity.z);
		float boosted = std::min(speed2d * k_driftReleaseBoost, m_maxSpeed);
		m_velocity.x = m_moveVec.x * boosted;
		m_velocity.z = m_moveVec.z * boosted;
		m_speed = boosted;

		if ((int(m_level) + 1) < (int)SpeedLevel::Clash)
		{
			ChangeSpeedLevel(SpeedLevel(int(m_level) + 1));

			GLOBALEVENT.publish(Events::Player::DriftResult(Events::Player::DriftResult::DriftResultType::Success));
		}
	}
	m_wasDrifting = m_isDrifting;

	// ドリフト中は低グリップ(慣性で横滑り)、通常は高グリップ
	float grip = m_isDrifting ? m_driftFriction : m_lateralFriction;
	float lerpT = std::min(grip * dt, 1.0f);

	Math::Vector3 targetVelocity = { m_moveVec.x * m_speed, 0.0f, m_moveVec.z * m_speed };
	m_velocity.x += (targetVelocity.x - m_velocity.x) * lerpT;
	m_velocity.z += (targetVelocity.z - m_velocity.z) * lerpT;

	// ── ドリフト角(カメラ・エフェクト用) ────────
	if (m_velocity.LengthSquared() > 0.0001f)
	{
		Math::Vector3 velDir = { m_velocity.x, 0.0f, m_velocity.z };
		velDir.Normalize();
		Math::Vector3 fwdDir = m_moveVec;
		fwdDir.y = 0.0f;
		fwdDir.Normalize();
		m_driftAngle = std::acos(std::clamp(fwdDir.Dot(velDir), -1.0f, 1.0f));
	}
	else
	{
		m_driftAngle = 0.0f;
	}

	// ==========================================
	// 6. 移動量を確定
	// ==========================================

	m_amountMove.x = m_velocity.x * dt;
	m_amountMove.z = m_velocity.z * dt;

	// 重力加速
	m_fallVelocity -= GRAVITY_ACCEL * dt;
	m_fallVelocity = std::max(m_fallVelocity, MAX_FALL_SPEED);
	m_fallDistance = m_fallVelocity * dt;
	m_amountMove.y = m_fallDistance;

	// ==========================================
	// 7. 速度の自然減衰
	// ==========================================

	// ドリフト中は減衰を弱めて速度を維持しやすくする
	float friction = m_isDrifting ? 0.35f : ((brakePressed ? 1.5f : (m_speed < 0.0f) ? 0.995f : 0.55f));
	m_speed *= std::exp(-friction * dt);

	if (std::abs(m_speed) < 0.01f)
	{
		m_speed = 0.0f;
		m_velocity = Math::Vector3::Zero;
		m_isDrifting = false;
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
		if (m_amountMove.y + m_pos.y < hitPos.y)
			m_amountMove.y = hitPos.y - m_pos.y;

		m_fallVelocity = 0.0f;
		m_fallDistance = 0.0f;
	}
}

// =============================================================
// 壁との衝突処理
// =============================================================

void Player::UpdateWallCollision(const std::vector<std::shared_ptr<KdGameObject>>& objList, const Math::Matrix& rotMat)
{
	std::set<std::shared_ptr<KdGameObject>> hitObjectsThisFrame;

	Math::Vector3 slidingVelocity = m_amountMove;
	float savedGravityY = slidingVelocity.y;
	slidingVelocity.y = 0.0f;

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
					hitObjectsThisFrame.insert(obj);
			}
		}

		if (retSphereList.empty()) break;

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
				if (push.dir.Dot(dir) > 0.9f)
				{
					isDuplicate = true;
					if (ret.m_overlapDistance > push.maxOverlap)
						push.maxOverlap = ret.m_overlapDistance;
					break;
				}
			}
			if (!isDuplicate)
				distinctPushes.push_back({ dir, ret.m_overlapDistance });
		}

		for (const auto& push : distinctPushes)
		{
			m_amountMove += push.dir * push.maxOverlap;

			// 壁衝突時にドリフト中の速度ベクトルも壁方向成分を除去
			float dotSliding = slidingVelocity.Dot(push.dir);
			if (dotSliding < 0.0f)
				slidingVelocity -= push.dir * dotSliding;

			float dotVelocity = m_velocity.Dot(push.dir);
			if (dotVelocity < 0.0f)
				m_velocity -= push.dir * dotVelocity;
		}

		m_amountMove.x = slidingVelocity.x;
		m_amountMove.z = slidingVelocity.z;
		m_amountMove.y = savedGravityY;
	}

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