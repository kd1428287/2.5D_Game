#include "CardBoard.h"

void CardBoard::Init()
{
	m_model = KdAssets::Instance().m_modeldatas.GetData("Asset/Models/Effect/Cardboard.gltf");

	m_pos = { 1.9,0.75f,1.2f };

	m_mWorld =
		Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_dir)) *
		Math::Matrix::CreateTranslation(m_pos);
}

void CardBoard::Update(float dt)
{
	m_rotAngle += ROT_SPEED * dt;

	m_mWorld =
		Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_dir + m_rotAngle)) *
		Math::Matrix::CreateTranslation(m_pos);
}

void CardBoard::GenerateDepthMapFromLight()
{
	if (!m_model) { return; }
	//KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld);
}

void CardBoard::DrawLit()
{
	if (!m_model) { return; }
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld);
}