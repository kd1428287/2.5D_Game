#include "ResultScene.h"
#include "../SceneManager.h"

#include "Application/System/InputManager/InputManager.h"
#include "Application/Object/ObjectManager/ObjectManager.h"

void ResultScene::Init()
{
	BaseScene::Init();

	// リザルト表示開始を通知（FadeManager が FadeOut を開始する）
	GLOBALEVENT.publish(Events::Else::ResultBegin());
	GLOBALEVENT.publish(Events::Else::FadeInBegin());

	m_state = InScene::GameToResult;

	// ゲーム終了イベントを受けて遷移フェーズへ移行
	m_fadeInSub = GLOBALEVENT.subscribe<Events::Else::FadeInCompleted>(
		[this](const Events::Else::FadeInCompleted&)
		{
			if (m_state == InScene::GameToResult)
			{
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

	if (m_state == InScene::ResultToTitle)
	{
		m_cnt += dt;

		if (m_cnt > 1.5f)
		{
			SceneManager::Instance().SetNextScene(SceneManager::SceneType::Game);
			GLOBALEVENT.publish(Events::Else::ResultToTitleEnd());
		}
	}

	if (m_state == InScene::GameToResult)
	{
		m_cnt += dt;

		if (m_cnt > 0.5f)
		{
			m_state = InScene::Result;
			m_cnt = 0;
			GLOBALEVENT.publish(Events::Else::DeliveryScoreRollBegin());
		}
	}

}