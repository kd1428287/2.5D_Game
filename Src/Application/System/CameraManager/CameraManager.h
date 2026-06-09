#pragma once

class Player;

class CameraManager
{
public:
	CameraManager() {};
	~CameraManager() {};

	void Init();
	void Update(float dt);

	void SetCameraTarget(std::shared_ptr<Player> targetObj) { m_targetObj = targetObj; };
	void SetCameraPos(Math::Vector3 camPos) { m_camPos = camPos; }
	void SetCameraAngleX(float xAng) { m_camAng.x = xAng; }
	void SetCameraAngleY(float yAng) { m_camAng.y = yAng; }
	void SetCameraAngleZ(float zAng) { m_camAng.z = zAng; }

	Math::Vector3 GetCameraPos() { return m_camPos; }
	Math::Vector3 GetCameraAngle() { return m_camAng; }


private:

	std::unique_ptr<KdCamera> m_camera = nullptr;
	const Math::Vector3 DEF_DIS = { 0.0f,0.5f,-0.5f };
	Math::Vector3 m_camDis;
	Math::Vector3 m_camPos;
	Math::Vector3 m_camAng;
	float m_projection = 0.0f;
	float m_speed = 0.0f;
	std::weak_ptr<Player> m_targetObj = {};

	float m_targetProj = 0.0f;
	Math::Vector3 m_targetPos;
	float m_targetAspectRatio = 0.0f;

	ScopedSubscriber m_subscriber;
};