#pragma once

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
	const std::list<std::shared_ptr<KdGameObject>>& GetObjList()
	{
		return m_objList;
	}
	
	// オブジェクトリストに追加
	void AddObject(const std::shared_ptr<KdGameObject>& _obj)
	{
		m_objList.push_back(_obj);
	}

	// カメラ座標/角度セット
	void SetCameraPos(Math::Vector3 camPos) { m_camTargetPos = camPos; }
	void SetCameraPos(KdGameObject* camTarget) { m_camTargetObj = camTarget; }
	void SetCameraAngle(float xAng, float yAng, float zAng) { m_camAng = Math::Vector3(xAng, yAng, zAng); }

protected :

	// 継承先シーンで必要ならオーバーライドする
	virtual void Event();
	virtual void Init();

	void UpdateCamera(float dt);

	std::unique_ptr<KdCamera> m_camera = nullptr;
	Math::Vector3 m_camPos;
	KdGameObject* m_camTargetObj = nullptr;
	Math::Vector3 m_camTargetPos;
	Math::Vector3 m_camAng;

	// 全オブジェクトのアドレスをリストで管理
	std::list<std::shared_ptr<KdGameObject>> m_objList;
};
