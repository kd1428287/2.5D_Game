#pragma once

struct CameraData
{
	Math::Vector3 pos;
	Math::Vector3 angle;
};


class Reader
{
public:
	static Reader& Instance()
	{
		static Reader instance;
		return instance;
	}
	void Init();
	void WriteCamera(CameraData data){ m_cameraData = data; }
	void WriteTime(float time) { m_time = time; }
	CameraData ReadCamera() { return m_cameraData; }
	float ReadTime() { return m_time; }

private:
	Reader() {};
	~Reader() {};
private:

	CameraData m_cameraData;
	float m_time = 0.f;
};