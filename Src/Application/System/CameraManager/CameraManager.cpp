#include "CameraManager.h"

#include "Application/Object/Character/Player/Player.h"


void CameraManager::Init()
{
	m_camDis = { 0.0f,3.0f,-3.0f };
	m_camAng.x = 30.0f;
	m_projection = 60.0f;
	m_camera = std::make_unique<KdCamera>();
	m_camera->SetProjectionMatrix(m_projection);
	m_speed = 0.5f;
}

void CameraManager::Update(float dt)
{
	std::shared_ptr<Player> _targetObj = nullptr;
	if (!m_targetObj.expired()) _targetObj = m_targetObj.lock();
	static float currentSteerOffset = 0.0f;
	float steerInput = _targetObj->GetSteeringInput();
	float targetOffset = steerInput * -15.0f; // 目標角度

	// 補間（0.1f は環境により調整。本来は dt を使うのが理想）
	float t = 0.1f * dt;
	currentSteerOffset = std::lerp(currentSteerOffset, targetOffset, t);

	// 2. ターゲット行列から位置を補間して取得
	Math::Matrix targetMat = _targetObj->GetMatrix();
	Math::Vector3 targetPos = targetMat.Translation();
	m_camPos = Math::Vector3::Lerp(m_camPos, targetPos, m_speed);

	// 3. カメラの姿勢を計算
	// 「車自体の向き(targetMat)」に対し、「カメラの基本角度(m_camAng)」と「オフセット(currentSteerOffset)」を加える
	Math::Matrix mat =
		Math::Matrix::CreateRotationX(DirectX::XMConvertToRadians(m_camAng.x)) *
		Math::Matrix::CreateTranslation(m_camDis) *
		// ここで (カメラの基本Y回転 + ステアリングによるYオフセット) を計算
		Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_camAng.y + currentSteerOffset)) *
		targetMat;

	m_camera->SetCameraMatrix(mat);
	m_camera->SetToShader();
}
