#pragma once
#include <Framework/GameObject/KdGameObject.h>

class CameraManager
{
public:
	CameraManager() {};
	~CameraManager() {};

	void Init();
	void Update(float dt);

	void SetCameraTarget(KdGameObject* targetObj) { m_targetObj = targetObj; };
	void SetCameraPos(Math::Vector3 camPos) { m_camPos = camPos; }
	void SetCameraAngleX(float xAng) { m_camAng.x = xAng; }
	void SetCameraAngleY(float yAng) { m_camAng.y = yAng; }
	void SetCameraAngleZ(float zAng) { m_camAng.z = zAng; }


private:
	

	std::unique_ptr<KdCamera> m_camera = nullptr;
	Math::Vector3 m_camPos;
	Math::Vector3 m_camAng;
	float m_projection = 0.0f;
	KdGameObject* m_targetObj = nullptr;
};