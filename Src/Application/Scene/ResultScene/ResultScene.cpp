#include "ResultScene.h"
#include "../SceneManager.h"

#include "Application/System/InputManager/InputManager.h"
#include "Application/Object/ObjectManager/ObjectManager.h"

#include "Application/Object/Character/AutoPlayer/AutoPlayer.h"
#include "Application/Object/SkySphere/SkySphere.h"

void ResultScene::Init()
{
	BaseScene::Init();

	// プレイヤー生成
	//const Math::Vector3 startPos = { 1.6f, 0.2f, 0.8f };
	//const Math::Vector3 startPos = { 2.4f, 0.2f, 0.79f };
	const Math::Vector3 startPos = { 3.2f, 0.2f, 0.8f };
	m_player = m_objectManager->CreateObject<AutoPlayer>(startPos);

	// マップ生成
	m_mapManager->GenerateResultMap(*m_objectManager);
	m_objectManager->CreateObject<SkySphere>();

	// リザルト表示開始を通知（FadeManager が FadeOut を開始する）
	GLOBALEVENT.publish(Events::Else::ResultBegin());
	GLOBALEVENT.publish(Events::Else::FadeInBegin());

	//GLOBALEVENT.publish(Events::Else::CreateObjectEvent("Number", Math::Vector3{ 0,0.5,0 }));

	m_state = InScene::GameToResult;

	// ゲーム終了イベントを受けて遷移フェーズへ移行
	m_fadeInSub = GLOBALEVENT.subscribe<Events::Else::FadeInCompleted>(
		[this](const Events::Else::FadeInCompleted&)
		{
			if (m_state == InScene::GameToResult)
			{
				std::vector<Math::Vector3> vec;
				vec.push_back({ 1.6f,0.1f,0.8f });
				m_player->StartAutoPilot(vec);
				m_state = InScene::Result;
				GLOBALEVENT.publish(Events::Else::GameToResultEnd());
			}
		});

	// ゲーム終了イベントを受けて遷移フェーズへ移行
	m_fadeOutSub = GLOBALEVENT.subscribe<Events::Else::FadeOutCompleted>(
		[this](const Events::Else::FadeOutCompleted&)
		{
			if (m_state == InScene::ResultToTitle)
			{
				SceneManager::Instance().SetNextScene(SceneManager::SceneType::Game);
			}
		});
}

void ResultScene::Event(float dt)
{
	// リターンキーでタイトルへ戻る
	if (InputManager::Instance().IsTriggered(VK_RETURN))
	{
		if (m_state == InScene::Result)
		{
			// タイトルへの暗転開始を通知（FadeManager が FadeIn を開始する）
			GLOBALEVENT.publish(Events::Else::ResultEnd());
			GLOBALEVENT.publish(Events::Else::ResultToTitleBegin());
			GLOBALEVENT.publish(Events::Else::FadeOutBegin());
			m_state = InScene::ResultToTitle;
		}
	}
}