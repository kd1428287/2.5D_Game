#include "Player.h"

#include "Application/System/InputManager/InputManager.h"

void Player::Init()
{
	m_polygon = std::make_shared<KdSquarePolygon>();

	m_polygon->SetMaterial("Asset/Textures/Character/Player/player.png");

	m_polygon->SetPivot(KdSquarePolygon::PivotType::Center_Bottom);

	m_polygon->SetScale(2);

	
}

void Player::Update(float dt)
{
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
	if (InputManager::Instance().IsTriggered(VK_LBUTTON))
	{
		m_state = PlayerState::Attack_start;
	}
}
