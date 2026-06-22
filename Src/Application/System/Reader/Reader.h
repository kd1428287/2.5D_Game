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
	void Init() {};
	void WriteCamera(CameraData data){ m_cameraData = data; }
	void WriteTime(float time) { m_time = time; }
	void WriteScore(Math::Vector3 score) { m_score = score; }
	void WriteScoreForPrd(int score) { m_scoreForProduction = score; }

	CameraData& ReadCamera() { return m_cameraData; }
	float ReadTime() { return m_time; }
	Math::Vector3 ReadScore() { return m_score; }
	int ReadScoreForPrd() { return m_scoreForProduction; }

private:
	Reader() {};
	~Reader() {};
private:
	CameraData m_cameraData = {};
	float m_time = 0.f;
	Math::Vector3 m_score = {};
	int m_scoreForProduction = 0;
};