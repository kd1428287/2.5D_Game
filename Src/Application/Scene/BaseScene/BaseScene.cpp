#include "BaseScene.h"

#include "Application/System/CameraManager/CameraManager.h"
#include "Application/System/UIManager/UIManager.h"
#include "Application/Object/ObjectManager/ObjectManager.h"
#include "../../main.h"

void BaseScene::PreUpdate()
{
	m_objectManager->PreUpdate();
}

void BaseScene::Update()
{
	float deltaTime = Application::Instance().GetDeltaTime();

	// シーン毎のイベント処理
	Event(deltaTime);
	m_objectManager->Update(deltaTime);
}

void BaseScene::PostUpdate()
{
	m_objectManager->PostUpdate();

	m_cameraManager->Update(Application::Instance().GetDeltaTime());
}

void BaseScene::PreDraw()
{
	m_objectManager->PreDraw();
}

void BaseScene::Draw()
{
	// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
	// 光を遮るオブジェクト(影を生み出す要因となるオブジェクト)をBeginとEndの間にまとめてDrawする
	KdShaderManager::Instance().m_StandardShader.BeginGenerateDepthMapFromLight();
	{
		m_objectManager->GenerateDepthMapFromLight();
		UIManager::Instance().GenerateDepthMapFromLight();
	}
	KdShaderManager::Instance().m_StandardShader.EndGenerateDepthMapFromLight();

	// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
	// 陰影のないオブジェクト(背景など)はBeginとEndの間にまとめてDrawする
	KdShaderManager::Instance().m_StandardShader.BeginUnLit();
	{
		m_objectManager->DrawUnLit();
	}
	KdShaderManager::Instance().m_StandardShader.EndUnLit();

	// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
	// 陰影のあるオブジェクト(光源の影響を受けるオブジェクト)はBeginとEndの間にまとめてDrawする
	KdShaderManager::Instance().m_StandardShader.BeginLit();
	{
		m_objectManager->DrawLit();
		
		UIManager::Instance().DrawLit();
	}
	KdShaderManager::Instance().m_StandardShader.EndLit();

	// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
	// 陰影のないオブジェクト(エフェクトなど)はBeginとEndの間にまとめてDrawする
	KdShaderManager::Instance().m_StandardShader.BeginUnLit();
	{
		m_objectManager->DrawEffect();
	}
	KdShaderManager::Instance().m_StandardShader.EndUnLit();

	// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
	// 光源オブジェクト(自ら光るオブジェクトやエフェクト)はBeginとEndの間にまとめてDrawする
	KdShaderManager::Instance().m_postProcessShader.BeginBright();
	{
		m_objectManager->DrawBright();
	}
	KdShaderManager::Instance().m_postProcessShader.EndBright();
}

void BaseScene::DrawSprite()
{
	// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
	// 2Dの描画はこの間で行う
	KdShaderManager::Instance().m_spriteShader.Begin();
	{
		m_cameraManager->DrawSprite();
		m_objectManager->DrawSprite();
		KdShaderManager::Instance().m_spriteShader.SetMatrix(Math::Matrix::Identity);
	}
	KdShaderManager::Instance().m_spriteShader.End();
}

void BaseScene::DrawDebug()
{
	// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
	// デバッグ情報の描画はこの間で行う
	KdShaderManager::Instance().m_StandardShader.BeginUnLit();
	{
		m_objectManager->DrawDebug();
	}
	KdShaderManager::Instance().m_StandardShader.EndUnLit();
}

const std::vector<std::shared_ptr<KdGameObject>>& BaseScene::GetObjList()
{
	return m_objectManager->GetObjList();
}

void BaseScene::AddObject(const std::shared_ptr<KdGameObject>& _obj)
{
	m_objectManager->AddObject(_obj);
}

void BaseScene::Event(float dt)
{
	// 各シーンで必要な内容を実装(オーバーライド)する
}

void BaseScene::Init()
{
	m_cameraManager = std::make_unique<CameraManager>();
	m_cameraManager->Init();
	m_objectManager = std::make_unique<ObjectManager>();
	m_objectManager->Init();
	m_mapManager = std::make_unique<MapManager>();
	m_mapManager->Init();
}