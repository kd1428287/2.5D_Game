#include "CameraManager.h"

#include "Application/Object/Character/Player/Player.h"


void CameraManager::Init()
{
	m_camDis = { 0.0f,3.0f,-3.0f };
	m_camAng.x = 25.0f;
	m_projection = 60.0f;
	m_camera = std::make_unique<KdCamera>();
	m_camera->SetProjectionMatrix(m_projection,2000.f,2.f);
	m_speed = 0.5f;

	KdShaderManager::Instance().WorkAmbientController().SetFogEnable(false, true);
	KdShaderManager::Instance().WorkAmbientController().SetheightFog({ 1,1,1 }, 0, 2, 0);
}

void CameraManager::Update(float dt)
{
	std::shared_ptr<Player> _targetObj = nullptr;
	if (!m_targetObj.expired()) _targetObj = m_targetObj.lock();
	static float currentSteerOffset = 0.0f;
	float steerInput = _targetObj->GetSteeringInput();
	float targetOffset = steerInput * -15.0f; // 目標角度

	// 補間
	float t = 1.5f * dt;
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
