#include "AutoPlayer.h"

#include "Application/System/CameraManager/CameraManager.h"
#include "Application/Scene/SceneManager.h"
#include "Application/System/Reader/Reader.h"


// =============================================================
// 初期化
// =============================================================

AutoPlayer::~AutoPlayer()
{
	Reader::Instance().WriteScoreForPrd(0);
}

void AutoPlayer::Init()
{
	m_pCollider = std::make_unique<KdCollider>();

	m_model = KdAssets::Instance().m_modeldatas.GetData("Asset/Models/car_van/car_van2.gltf");

	m_speed = 0.0f;
	if (m_scale == Math::Vector3::Zero) m_scale = { 1.0f, 1.0f, 1.0f };
	m_level = SpeedLevel::Idle;

	ChangeSpeedLevel(SpeedLevel::Idle);
	m_acceleration = 5.0f;

	auto score = Reader::Instance().ReadScore();
	m_deliveryScore = score.x;
	m_deliveryDestroy = score.y;
	m_destroyScore = score.z;

	// --- イベント購読 ---

	// 衝突結果（エフェクトやクラッシュ反応のみ残す）
	m_subscriber.push_back(
		GLOBALEVENT.subscribe<Events::Player::HitResult>([this](const Events::Player::HitResult& e)
			{
				if (e.type == Events::Player::HitResult::HitResultType::Ignored) return;

				int level = (int)m_level - 2;

				switch (e.type)
				{
				case Events::Player::HitResult::HitResultType::Destroyed:
					return; // スコア加算は削除

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

	// 配達完了（演出のアニメーションのみ残す）
	m_subscriber.push_back(
		GLOBALEVENT.subscribe<Events::Player::DeliveryPointCompleted>([this](const Events::Player::DeliveryPointCompleted& e)
			{
				m_isDeliveryAnime = true;
				m_deliveryAnimeTime = 0.0f;
			})
	);

	// スピードアップアイテム取得（任意で加速）
	m_subscriber.push_back(
		GLOBALEVENT.subscribe<Events::Player::GetSpeedUp>([this](const Events::Player::GetSpeedUp& e)
			{
				if (m_level >= SpeedLevel::Speed6) return;
				ChangeSpeedLevel((SpeedLevel)((int)m_level + 1));
			})
	);

	m_pCollider->RegisterCollisionShape("PlayerCollision", m_model, KdCollider::Type::TypeEvent);
}

// =============================================================
// 毎フレーム更新
// =============================================================

void AutoPlayer::PreUpdate()
{
	m_amountMove = Math::Vector3::Zero;
}

void AutoPlayer::Update(float dt)
{
	if (!m_isActive) return;

	UpdateMove(dt);
	UpdateAction(dt);

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

void AutoPlayer::PostUpdate()
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

void AutoPlayer::GenerateDepthMapFromLight()
{
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld);
}

void AutoPlayer::DrawLit()
{
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld);
}

// =============================================================
// スピードレベル変更
// =============================================================

void AutoPlayer::ChangeSpeedLevel(SpeedLevel level)
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

	// 必要に応じてイベント通知を残すか削除するか選択可能
	GLOBALEVENT.publish(Events::Player::ChangeSpeedLevel((int)m_level));
}

// =============================================================
// 自動操縦 API
// =============================================================

void AutoPlayer::StartAutoPilot(const std::vector<Math::Vector3>& waypoints)
{
	m_isActive = true;
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

void AutoPlayer::StopAutoPilot()
{
	//m_isActive = false;
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
// 自動操縦の仮想入力計算
// =============================================================

void AutoPlayer::UpdateAutoPilotInput(float dt)
{
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
			if (m_deliveryScore > 0)
			{
				m_isAction = true;
				m_actionWait = 0.5f;
			}
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

void AutoPlayer::UpdateAction(float dt)
{
	if (!m_isAction)
	{
		return;
	}
	m_actionWait -= dt;
	if (m_actionWait <= 0.f)
	{
		m_isDeliveryAnime = true;
		m_deliveryAnimeTime = 0.0f;
		m_deliveryScore--;
		Reader::Instance().WriteScoreForPrd(Reader::Instance().ReadScoreForPrd() + 1);

		m_actionWait = 0.5;
		GLOBALEVENT.publish(Events::Else::ResultPlayerProduction(Events::Else::ResultPlayerProduction::State::Dispatch));
		if (m_deliveryScore <= 0)
		{
			m_isAction = false;
			GLOBALEVENT.publish(Events::Else::ResultPlayerProduction(Events::Else::ResultPlayerProduction::State::Completed));
		}
	}

}

// =============================================================
// 移動処理
// =============================================================

void AutoPlayer::UpdateMove(float dt)
{
	// 自動操縦の入力を更新
	UpdateAutoPilotInput(dt);

	bool accelPressed = (m_autoAccelInput > 0.0f);
	bool brakePressed = (m_autoAccelInput < 0.0f);

	// ==========================================
	// 1. アクセル / ブレーキ
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
	// 2. 車の向きの更新（バイシクルモデル）
	// ==========================================
	// 自動操縦側で m_angle.y を直接書き換えている場合 m_steering は 0 のため変化しないが、
	// 補間式アルゴリズムに変更した時のために計算式は残す
	if (std::abs(m_speed) > 0.01f && m_steering != 0.0f)
	{
		float steerRad = DirectX::XMConvertToRadians(m_steering);
		float angularVelocity = (m_speed * std::tan(steerRad)) / m_wheelBase;
		m_angle.y += DirectX::XMConvertToDegrees(angularVelocity * dt);
	}

	// ==========================================
	// 3. 移動ベクトルの計算
	// ==========================================
	float pitch_rad = DirectX::XMConvertToRadians(m_angle.x);
	float yaw_rad = DirectX::XMConvertToRadians(m_angle.y);

	// 水平推進ベクトル
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
	// 4. 速度の自然減衰（摩擦・空気抵抗）
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

void AutoPlayer::UpdateGroundCollision(const std::vector<std::shared_ptr<KdGameObject>>& objList)
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
		{
			m_amountMove.y = hitPos.y - m_pos.y;
		}
		m_fallVelocity = 0.0f;
		m_fallDistance = 0.0f;
	}
}

// =============================================================
// 壁との衝突処理
// =============================================================

void AutoPlayer::UpdateWallCollision(const std::vector<std::shared_ptr<KdGameObject>>& objList, const Math::Matrix& rotMat)
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
				{
					hitObjectsThisFrame.insert(obj);
				}
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
			m_amountMove += push.dir * push.maxOverlap;

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