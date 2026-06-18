#include "inc_KdPostProcessShader.hlsli"

Texture2D g_inputTex : register(t0);
SamplerState g_ss : register(s0);

cbuffer cb : register(b0)
{
	float g_intensity;		// ブラー全体の強さ：0で無効、1で最大
	float g_innerRadius;	// ここから内側はブラーがかからない：0.0～1.0(画面半径に対する割合)
	float g_outerRadius;	// ここで最大強度になる：0.0～1.0(画面半径に対する割合)
	int   g_samplingNum;	// 中心方向へのサンプリング回数
};

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 高速移動時に画面外周へかける速度ブラー(ラジアルブラー)シェーダー
// 画面中心からの距離に応じてブラー強度を補間し、中心方向に向かって複数回サンプリングする
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
float4 main(VSOutput In) : SV_Target0
{
	// アスペクト比を考慮せず、UV空間(0～1)上での中心からの距離で判定する
	float2 center = float2(0.5f, 0.5f);
	float2 diff = In.UV - center;

	// 画面の角までの距離(中心からの最大距離)を基準に正規化
	float dist = length(diff) / length(center);

	// Inner～Outerの間で0→1に補間し、さらに全体強度を掛ける
	float blurAmount = saturate((dist - g_innerRadius) / max(g_outerRadius - g_innerRadius, 0.0001f));
	blurAmount *= g_intensity;

	// ブラーがほぼ無い部分は早期に元の色を返す（負荷軽減）
	if (blurAmount <= 0.001f || g_samplingNum <= 0)
	{
		return g_inputTex.Sample(g_ss, In.UV);
	}

	// 中心方向への単位ベクトル
	float2 dirToCenter = normalize(center - In.UV);

	// サンプリングする最大オフセット距離（外側ほど大きくブラー）
	float maxOffset = blurAmount * 0.05f; // 0.05はブラーの最大強度の調整値

	float3 color = 0;
	float weightSum = 0.0f;

	[loop]
	for (int i = 0; i < g_samplingNum; i++)
	{
		// 0 ～ maxOffset の範囲で段階的にサンプリング位置をずらす
		float t = (float)i / max((float)(g_samplingNum - 1), 1.0f);
		float2 offset = dirToCenter * (t * maxOffset);

		// 中心に近いサンプルほど重みを大きくする(ガウシアン的な重み付け)
		float weight = exp(-t * t * 4.0f);

		color += g_inputTex.Sample(g_ss, In.UV + offset).rgb * weight;
		weightSum += weight;
	}

	color /= weightSum;

	return float4(color, 1);
}
