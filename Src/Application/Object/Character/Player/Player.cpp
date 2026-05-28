#include "Player.h"

#include "Application/System/InputManager/InputManager.h"

void Player::Init()
{
	// テクスチャ or モデル
	m_polygon = std::make_shared<KdSquarePolygon>();

	m_polygon->SetMaterial("Asset/Textures/Character/Player/player.png");

	m_polygon->SetPivot(KdSquarePolygon::PivotType::Center_Bottom);

	m_polygon->SetScale(2);

	
	m_speed = 0.2f;
}

void Player::PreUpdate()
{
	m_moveVec = Math::Vector3::Zero;
}

void Player::Update(float dt)
{
	ActiveInput();

	m_moveVec.Normalize();
	m_pos += m_moveVec * m_speed;

	m_mWorld = Math::Matrix::CreateTranslation(m_pos);
}

void Player::PostUpdate()
{

}

void Player::GenerateDepthMapFromLight()
{
	KdShaderManager::Instance().m_StandardShader.DrawPolygon(*m_polygon, m_mWorld);
}

void Player::DrawLit()
{
	KdShaderManager::Instance().m_StandardShader.DrawPolygon(*m_polygon, m_mWorld);
}

void Player::ActiveInput()
{
	/*if (InputManager::Instance().IsTriggered(VK_LBUTTON))
	{
		m_state = PlayerState::Attack_start;
	}*/

	if (InputManager::Instance().IsPressed(VK_UP))
	{
		m_moveVec.z = 1;
	}
	if (InputManager::Instance().IsPressed(VK_LEFT))
	{
		m_moveVec.x = -0.2f;
	}
	if (InputManager::Instance().IsPressed(VK_DOWN))
	{
		m_moveVec.z = -1;
	}
	if (InputManager::Instance().IsPressed(VK_RIGHT))
	{
		m_moveVec.x = 0.2f;
	}
}
