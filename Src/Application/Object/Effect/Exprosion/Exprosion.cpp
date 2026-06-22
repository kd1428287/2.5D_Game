#include "Exprosion.h"
#include "../../../System/Reader/Reader.h"

void Exprosion::Init()
{
	m_polygon = std::make_shared<KdSquarePolygon>(KdAssets::Instance().m_textures.GetData("Asset/Textures/Effect/pipo-btleffect022.png"));
	//m_polygon = std::make_shared<KdSquarePolygon>(KdAssets::Instance().m_textures.GetData("Asset/Textures/Effect/pipo-mapeffect005.png"));
	m_polygon->SetSplit(8, 1);
	m_animeParam.startIdx = 0;
	m_animeParam.endIdx = 8;
	m_animeParam.animeIdx = m_animeParam.startIdx;
	m_animeParam.cntSpeed = 10.f;

	//for (int i = 0; i < 5; i++)
	{
		GLOBALEVENT.publish(Events::Else::CreateObjectEvent("Smoke", m_pos, 0.3f, false, 40.f));
	}
}

void Exprosion::Update(float dt)
{
	m_animeParam.animeIdx += m_animeParam.cntSpeed * dt;
	if (m_animeParam.animeIdx >= m_animeParam.endIdx)
	{
		if (m_loopFlg)
		{
			m_animeParam.animeIdx = m_animeParam.startIdx;
		}
		else
		{
			m_isExpired = true;
			return;
		}
	}

	m_polygon->SetUVRect((int)m_animeParam.animeIdx);
}

void Exprosion::PostUpdate()
{
	auto data = Reader::Instance().ReadCamera();

	Math::Vector3 scale, translation;
	Math::Quaternion rotation;

	// Decomposeで分解
	data.mat.Decompose(scale, rotation, translation);

	// 再合成
	m_mWorld = Math::Matrix::CreateScale(m_scale)
		* Math::Matrix::CreateFromQuaternion(rotation)  // ← srcの回転
		* Math::Matrix::CreateTranslation(m_pos);
}

void Exprosion::DrawEffect()
{
	KdShaderManager::Instance().m_StandardShader.DrawPolygon(*m_polygon, m_mWorld,kWhiteColor,Math::Vector3{0,0,0});
}
