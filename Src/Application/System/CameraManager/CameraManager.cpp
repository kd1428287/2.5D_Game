#include "CameraManager.h"

void CameraManager::Init()
{
	m_camPos = { 0.0f,3.0f,-3.0f };
	m_camAng.x = 30.0f;
	m_projection = 60.0f;
	m_camera = std::make_unique<KdCamera>();
	m_camera->SetProjectionMatrix(m_projection);
}

void CameraManager::Update()
{
	Math::Matrix targetMat = Math::Matrix::Identity;
	if (m_targetObj)targetMat = m_targetObj->GetMatrix();
	Math::Matrix mat = 
		(Math::Matrix::CreateRotationX(DirectX::XMConvertToRadians(m_camAng.x)) *
			
		Math::Matrix::CreateTranslation(m_camPos)) * 
		Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_camAng.y)) *
		targetMat;

	//Math::Matrix::CreateLookAt

	m_camera->SetCameraMatrix(mat);

	m_camera->SetToShader();
}
