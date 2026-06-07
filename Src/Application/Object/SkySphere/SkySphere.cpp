#include "SkySphere.h"

void SkySphere::Init()
{
	m_model = std::make_shared<KdModelData>();
	m_model = RESOURCE.GetModel("Asset/Models/SkySphere/skySphere.gltf");

	m_mWorld =
		Math::Matrix::CreateScale(15) *
		Math::Matrix::CreateTranslation({ 0,-10,0 });
}

void SkySphere::DrawUnLit()
{
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld);
}
