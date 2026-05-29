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
	m_speed = 0.2f;
}

void Player::PreUpdate()
{
	m_nowVec.Normalize();
	m_moveVec = m_nowVec;
}

void Player::Update(float dt)
{
	ActiveInput();

	m_moveVec.Normalize();
	m_pos += m_moveVec * m_speed;
	m_speed *= 0.01f;
	if (m_speed > 0.0f)m_speed = 0.0f;

	m_mWorld = Math::Matrix::CreateTranslation(m_pos);
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
		m_moveVec.x = -0.2f;
	}
	if (InputManager::Instance().IsPressed(VK_RIGHT))
	{
		m_moveVec.x = 0.2f;
	}
}
