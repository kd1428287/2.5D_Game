#pragma once

struct CameraData
{
	Math::Matrix mat;
	std::weak_ptr<KdCamera> m_camera;
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
	CameraData& ReadCamera() { return m_cameraData; }
	float ReadTime() { return m_time; }

private:
	Reader() {};
	~Reader() {};
private:

	CameraData m_cameraData;
	float m_time = 0.f;
};