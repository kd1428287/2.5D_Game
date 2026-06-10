#include "CameraManager.h"

#include "Application/Object/Character/Player/Player.h"


void CameraManager::Init()
{
	m_camDis = DEF_DIS;
	m_camAng.x = 25.0f;
	m_projection = 60.0f;
	m_targetAngle = -20.f;
	m_targetProj = m_projection;
	m_targetPos = m_camDis;
	m_speed = 1.5f;

	m_camera = std::make_unique<KdCamera>();
	m_camera->SetProjectionMatrix(m_projection,2000.f,0.05f);
	

	KdShaderManager::Instance().WorkAmbientController().SetFogEnable(false, true);
	KdShaderManager::Instance().WorkAmbientController().SetheightFog({ 1,1,1 }, 0, 2, 0);

	m_speedSub = GLOBALEVENT.subscribe<Events::Player::ChangeSpeedLevel>([this](const Events::Player::ChangeSpeedLevel& e)
		{
			if (e.level != (int)SpeedLevel::Clash)
			{
				m_targetProj = 60.f + e.level * 12.f;
			}
			else
			{
				m_targetProj = 60.f;
			}
		});

	m_hitSub = GLOBALEVENT.subscribe<Events::Player::HitResult>([this](const Events::Player::HitResult& e)
		{
			switch (e.type)
			{
			case Events::Player::HitResult::HitResultType::Destroyed:
				// シェイク開始（例：強さ0.025f, 時間0.3秒）
				m_shakeStrength = 0.025f;
				m_shakeTime = 0.3f;
				return;
			case Events::Player::HitResult::HitResultType::Bounced:
				if (e.speedLevel <= 3 || e.speedLevel == 6)return;
				m_shakeStrength = 0.025f + 0.025f * (e.speedLevel - 3);
				m_shakeTime = 0.3f;
				return;
			default:
				return;
			}
		});
}

void CameraManager::Update(float dt)
{
	std::shared_ptr<Player> _targetObj = nullptr;
	if (!m_targetObj.expired()) _targetObj = m_targetObj.lock();

	UpdateProjection(dt);
	UpdateAngle(_targetObj, dt);
	UpdateDistance(_targetObj, dt);
	
	Math::Matrix targetMat = _targetObj->GetMatrix();
	m_camPos = targetMat.Translation();

	// シェイクの減衰処理
	Math::Vector3 shakeOffset = Math::Vector3::Zero;
	if (m_shakeTime > 0.0f)
	{
		m_shakeTime -= dt;
		// ランダムな方向に揺らす
		shakeOffset.x = KdRandom::GetFloat(-1.f, 1.f) * m_shakeStrength;
		shakeOffset.y = KdRandom::GetFloat(-1.f, 1.f) * m_shakeStrength;

		// 時間経過とともに弱める
		m_shakeStrength *= 0.9f;
	}

	// カメラの姿勢を計算
	// 「車自体の向き(targetMat)」に対し、「カメラの基本角度(m_camAng)」と「オフセット(currentSteerOffset)」を加える
	Math::Matrix mat =
		Math::Matrix::CreateRotationX(DirectX::XMConvertToRadians(m_camAng.x)) *
		Math::Matrix::CreateTranslation(m_camDis + shakeOffset) *
		// ここで (カメラの基本Y回転 + ステアリングによるYオフセット) を計算
		Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_camAng.y + m_steeringOffset)) *
		targetMat;

	m_camera->SetCameraMatrix(mat);
	m_camera->SetToShader();
}

void CameraManager::UpdateProjection(float dt)
{
	if (m_projection == m_targetProj)return;
	float t = m_speed * dt;
	m_projection = std::lerp(m_projection, m_targetProj, t);
	m_camera->SetProjectionMatrix(m_projection, 2000.f, 0.05f);
}

void CameraManager::UpdateAngle(const std::shared_ptr<Player>& target, float dt)
{
	float steerInput = target->GetSteeringInput();
	float targetOffset = steerInput * m_targetAngle; // 目標角度

	// 補間
	float t = m_speed * dt;
	m_steeringOffset = std::lerp(m_steeringOffset, targetOffset, t);

}

void CameraManager::UpdateDistance(const std::shared_ptr<Player>&target, float dt)
{
	float t = m_speed * dt;
	m_camDis = Math::Vector3::Lerp(m_camDis, m_targetPos, t);
}
