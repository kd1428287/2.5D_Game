#pragma once

class CameraManager
{
public:
	static CameraManager& Instance()
	{
		static CameraManager instance;
		return instance;
	}

	void Init();
	void Update();

	void SetCameraPos(Math::Vector3 camPos) { m_camPos = camPos; }
	void SetCameraAngle(float xAng, float yAng, float zAng) { m_camAng = Math::Vector3(xAng, yAng, zAng); }


private:
	CameraManager() {};
	~CameraManager() {};

	Math::Vector3 m_camPos;
	Math::Vector3 m_camAng;
};