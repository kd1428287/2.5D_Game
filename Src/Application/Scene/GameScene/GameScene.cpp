#include "GameScene.h"
#include "../SceneManager.h"

#include "Application/Object/Character/Player/Player.h"
#include "Application/Object/SkySphere/SkySphere.h"
#include "Application/System/InputManager/InputManager.h"
#include "Application/System/CameraManager/CameraManager.h"
#include "Application/Object/ObjectManager/ObjectManager.h"
#include "Application/Object/ObjectManager/MapManager/MapManager.h"

void GameScene::Init()
{
	BaseScene::Init();

	m_spawnManager = std::make_unique<SpawnManager>(m_objectManager);
	m_spawnManager->Init();
	m_timeManager = std::make_unique<TimeManager>();
	m_timeManager->Init();

	// プレイヤー生成
	const Math::Vector3 startPos = { 0.f, 0.1f, 0.f };
	auto player = m_objectManager->CreateObject<Player>(startPos);
	m_cameraManager->SetCameraTarget(player);

	// マップ生成
	auto map = m_mapManager->GenarateMap(*m_objectManager);
	m_spawnManager->SetMapData(map);

	m_objectManager->CreateObject<SkySphere>();

	// タイトルフェーズから開始
	GLOBALEVENT.publish(Events::Else::TitleBegin());
	m_state = InScene::Title;
	m_resultTransitionTimer = 0.f;

	// ゲーム終了イベントを受けて遷移フェーズへ移行
	m_endSub = GLOBALEVENT.subscribe<Events::Else::GameEnd>(
		[this](const Events::Else::GameEnd&)
		{
			if (m_state == InScene::Game)
			{
				// 暗転開始を通知し、遷移カウントを開始する
				GLOBALEVENT.publish(Events::Else::GameToResultBegin());
				m_state = InScene::GameToResult;
				m_resultTransitionTimer = 0.f;
			}
		});
}

void GameScene::Event(float dt)
{
	m_spawnManager->Update(dt);
	m_timeManager->Update(dt);

	// タイトル → ゲーム開始
	if (m_state == InScene::Title)
	{
		if (InputManager::Instance().IsTriggered(VK_RETURN))
		{
			GLOBALEVENT.publish(Events::Else::TitleToGameBegin());
			m_state = InScene::Game;
		}
	}

	// ゲーム → リザルト 遷移処理
	if (m_state == InScene::GameToResult)
	{
		UpdateGameToResult(dt);
	}

	if (InputManager::Instance().IsTriggered('T'))
	{
		GLOBALEVENT.publish(Events::Else::GameEnd());
		GLOBALEVENT.publish(Events::Else::GameToResultBegin());
	}
	
}

void GameScene::UpdateGameToResult(float dt)
{
	m_resultTransitionTimer += dt;

	// 暗転が完了する時間（秒）を待ってからシーン切り替え
	constexpr float kTransitionWaitSec = 1.5f;
	if (m_resultTransitionTimer >= kTransitionWaitSec)
	{
		// リザルトシーンへ移行
		// ※ ResultBegin は ResultScene::Init() 側で publish するため、ここでは発行しない
		GLOBALEVENT.publish(Events::Else::GameToResultEnd());
		SceneManager::Instance().SetNextScene(SceneManager::SceneType::Result);
	}
}