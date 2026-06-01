#include "Player.h"

#include "Application/System/InputManager/InputManager.h"
#include "Application/System/CameraManager/CameraManager.h"

void Player::Init()
{
	// モデル
	/*m_polygon = std::make_shared<KdSquarePolygon>();

	m_polygon->SetMaterial("Asset/Textures/Character/Player/player.png");

	m_polygon->SetPivot(KdSquarePolygon::PivotType::Center_Bottom);

	m_polygon->SetScale(2);*/

	m_model = std::make_shared<KdModelData>();
	m_model->Load("Asset/Models/car_van/car_van.gltf");
	//m_model->CreateMaterials

	CameraManager::Instance().SetCameraTarget(this);
	
	//m_pos.y = 3.0f;
	m_speed = 0.0f;
	m_angle = { 0,0,0 };
}

void Player::PreUpdate()
{
	
}

void Player::Update(float dt)
{
	ActiveInput();
	UpdateMove(dt);
	

	m_mWorld = 
		Math::Matrix::CreateScale(1.0f) *
		Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_angle.z)) *
		Math::Matrix::CreateRotationX(DirectX::XMConvertToRadians(m_angle.x)) *
		Math::Matrix::CreateTranslation(m_pos);
}

void Player::PostUpdate()
{

}

void Player::GenerateDepthMapFromLight()
{
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld);
}

void Player::DrawLit()
{
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld);
}

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
		m_speed += 0.05f;
	}
	if (InputManager::Instance().IsPressed(VK_DOWN))
	{
		m_speed -= 0.02f;
	}
	if (InputManager::Instance().IsPressed(VK_LEFT))
	{
		zRotation = -1.0f;
	}
	if (InputManager::Instance().IsPressed(VK_RIGHT))
	{
		zRotation = 1.0f;
	}

	float pitch_rad = DirectX::XMConvertToRadians(m_angle.x);
	float yaw_rad = DirectX::XMConvertToRadians(m_angle.z + zRotation);
	float x = std::cos(pitch_rad) * std::sin(yaw_rad);
	float y = std::sin(pitch_rad);
	float z = std::cos(pitch_rad) * std::cos(yaw_rad);
	m_moveVec = { x, y, z };

	CameraManager::Instance().SetCameraAngleZ(zRotation);

	m_moveVec.Normalize();
	m_pos += m_moveVec * m_speed;
	m_speed *= 0.01f * dt * 60.0f;
	if (m_speed > -0.005f && m_speed < 0.005f)m_speed = 0.0f;
	else m_angle.z += zRotation;
}
