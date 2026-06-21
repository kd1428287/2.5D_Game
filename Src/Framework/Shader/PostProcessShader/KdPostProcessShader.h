#pragma once

namespace Math = DirectX::SimpleMath;


class KdPostProcessShader
{
public:
	KdPostProcessShader() {}
	~KdPostProcessShader()
	{
		Release();
	}

	void SetNearClippingDistance(float distance) { m_cb0_DoFInfo.Work().NearClippingDistance = distance; }
	void SetFarClippingDistance(float distance) { m_cb0_DoFInfo.Work().FarClippingDistance = distance; }
	void SetFocusDistance(float distance) { m_cb0_DoFInfo.Work().FocusDistance = distance; }
	void SetFocusRange(float fore, float back) { m_cb0_DoFInfo.Work().FocusForeRange = fore; m_cb0_DoFInfo.Work().FocusBackRange = back; }

	void SetBrightThreshold(float threshold) { m_cb0_BrightInfo.Work().Threshold = threshold; }

	// 速度ブラー(画面外周にかけるラジアルブラー)関連設定
	// 速度の感知は行わず、ここで渡された値をそのまま使用する(呼び出し側が任意のタイミング・値で設定する想定)

	// ・intensity		… ブラー全体の強さ 0.0(無効)～1.0(最大)
	void SetSpeedBlurIntensity(float intensity) { m_cb0_SpeedBlurInfo.Work().Intensity = intensity; }

	// ・innerRadius	… ここから内側はブラーがかからない範囲　0.0～1.0
	// ・outerRadius	… ここで最大強度になる範囲　0.0～1.0(innerRadiusより大きい値)
	void SetSpeedBlurRange(float innerRadius, float outerRadius)
	{
		m_cb0_SpeedBlurInfo.Work().InnerRadius = innerRadius;
		m_cb0_SpeedBlurInfo.Work().OuterRadius = outerRadius;
	}

	// ・samplingNum	… 中心方向へのサンプリング回数(多いほど滑らかだが負荷が上がる)
	void SetSpeedBlurSamplingNum(int samplingNum) { m_cb0_SpeedBlurInfo.Work().SamplingNum = samplingNum; }

	// 現在設定されている速度ブラーパラメータの取得(必要であれば外部から参照可能)
	//const cbSpeedBlur& GetSpeedBlurParam() const { return m_cb0_SpeedBlurInfo.Get(); }

	struct Vertex
	{
		Math::Vector3 Pos;
		Math::Vector2 UV;
	};

	bool Init();

	void Release();

	void Draw();

	void BeginBright();
	void EndBright();

	void PostEffectProcess();

	void GenerateBlurTexture(std::shared_ptr<KdTexture>& spSrcTex, std::shared_ptr<KdTexture>& spDstTex, D3D11_VIEWPORT& VP, int blurRadius);

private:

	void BlurProcess();
	void LightBloomProcess();
	void DepthOfFieldProcess();
	void SpeedBlurProcess();

	void CreateBlurOffsetList(std::vector<Math::Vector3>& dstInfo, const std::shared_ptr<KdTexture>& spSrcTex, int samplingSize, const Math::Vector2& dir);

	void DrawTexture(std::shared_ptr<KdTexture>* spSrcTex, int srcTexSize, std::shared_ptr<KdTexture> spDstTex, D3D11_VIEWPORT* pVP);

	void SetBlurInfo(const std::shared_ptr<KdTexture>& spSrcTex, int samplingSize, const Math::Vector2& dir);
	void SetBlurInfo(const std::vector<Math::Vector3>& srcInfo);

	void SetBlurToDevice();
	void SetDoFToDevice();
	void SetBrightToDevice();
	void SetSpeedBlurToDevice();

	ID3D11VertexShader* m_VS = nullptr;
	ID3D11InputLayout* m_inputLayout = nullptr;

	ID3D11PixelShader* m_PS_Blur = nullptr;
	ID3D11PixelShader* m_PS_DoF = nullptr;
	ID3D11PixelShader* m_PS_Bright = nullptr;
	ID3D11PixelShader* m_PS_SpeedBlur = nullptr;

	static const int kBlurSamplingRadius = 8;
	static const int kLightBloomSamplingRadius = 4;

	static const int kMaxSampling = 31;
	struct cbBlur
	{
		Math::Vector4 Info[kMaxSampling];
	
		int SamplingNum = 0;
		int _blank[3] = { 0, 0 ,0 };
	};
	KdConstantBuffer<cbBlur>	m_cb0_BlurInfo;

	struct cbDepthOfField
	{
		float NearClippingDistance = 0.0f;
		float FarClippingDistance = 1000.0f;

		float FocusDistance = 0.0f;
		float FocusForeRange = 0.0f;
		float FocusBackRange = 1000.0f;
		int   _blank[3] = { 0, 0, 0 };
	};
	KdConstantBuffer<cbDepthOfField>	m_cb0_DoFInfo;

	struct cbBrightFilter
	{
		float Threshold = 0.0f;
		int _blank[3] = { 0, 0, 0 };
	};
	KdConstantBuffer<cbBrightFilter>	m_cb0_BrightInfo;

	// 速度ブラー(画面外周にかけるラジアルブラー)用定数バッファ
	struct cbSpeedBlur
	{
		float	Intensity = 0.0f;		// ブラー全体の強さ：0で無効、1で最大
		float	InnerRadius = 0.6f;		// ここから内側はブラーがかからない：0.0～1.0
		float	OuterRadius = 1.0f;		// ここで最大強度になる：0.0～1.0
		int		SamplingNum = 8;		// 中心方向へのサンプリング回数
	};
	KdConstantBuffer<cbSpeedBlur>	m_cb0_SpeedBlurInfo;

	KdRenderTargetPack	m_postEffectRTPack;

	KdRenderTargetPack	m_blurRTPack;
	KdRenderTargetPack	m_strongBlurRTPack;

	KdRenderTargetPack	m_depthOfFieldRTPack;

	KdRenderTargetPack	m_speedBlurRTPack;

	KdRenderTargetPack	m_brightEffectRTPack;
	static const int	kLightBloomNum = 4;
	KdRenderTargetPack	m_lightBloomRTPack[kLightBloomNum];

	KdRenderTargetChanger m_postEffectRTChanger;
	KdRenderTargetChanger m_brightRTChanger;

	Vertex m_screenVert[4];
};
