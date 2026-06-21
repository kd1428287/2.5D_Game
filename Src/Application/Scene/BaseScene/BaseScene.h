#pragma once

#include "Application/Object/ObjectManager/ObjectManager.h"
#include "Application/Object/ObjectManager/MapManager/MapManager.h"
#include "Application/Object/ObjectManager/SpawnManager/SpawnManager.h"
#include "Application/System/CameraManager/CameraManager.h"
#include "Application/System/EventBus/LocalEventBus.h"
#include "Application/System/AudioManager/AudioManager.h"

// GameScene 内部の進行フェーズ
enum class InScene
{
	Title,          // タイトル表示中（ゲーム開始待ち）
	Game,           // ゲームプレイ中
	GameToResult,   // 結果画面への遷移待ち（暗転中）
	Result,
	ResultToTitle,
};

class BaseScene
{
public :

	BaseScene()			 { Init(); }
	virtual ~BaseScene() {}

	void PreUpdate();
	void Update();
	void PostUpdate();

	void PreDraw();
	void Draw();
	void DrawSprite();
	void DrawDebug();

	// オブジェクトリストを取得
	const std::vector<std::shared_ptr<KdGameObject>>& GetObjList();
	
	
	// オブジェクトリストに追加
	void AddObject(const std::shared_ptr<KdGameObject>& _obj);

protected :

	// 継承先シーンで必要ならオーバーライドする
	virtual void Event(float dt);
	virtual void Init();

	// マネージャーを各シーンで保持
	std::unique_ptr<CameraManager>	m_cameraManager = nullptr;
	std::unique_ptr<ObjectManager>	m_objectManager = nullptr;
	std::unique_ptr<MapManager>		m_mapManager = nullptr;
};
