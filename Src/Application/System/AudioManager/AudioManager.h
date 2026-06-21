#pragma once

// ============================================================
// ループサウンド1つを管理するハンドル
// ・ピッチ・音量を目標値へ滑らかに補間する
// ・音量が0になったら自動的に停止できる（アイドル切り替え時などに使用）
// ============================================================
struct LoopSoundHandle
{
	std::shared_ptr<KdSoundInstance> instance;

	float currentVolume = 1.0f;
	float targetVolume = 1.0f;

	float currentPitch = 0.0f;   // DirectXTK: -1.0(半音下) 〜 0.0(等速) 〜 1.0(半音上)
	float targetPitch = 0.0f;

	float interpSpeed = 2.0f;   // 補間速度（毎秒この値だけ近づく）

	bool  stopWhenSilent = false;  // 音量が0に達したら自動停止するか

	bool IsValid() const { return instance != nullptr; }

	// 目標ピッチを設定（即時 or 補間）
	void SetTargetPitch(float pitch, bool immediate = false)
	{
		targetPitch = pitch;
		if (immediate && instance) { instance->SetPitch(pitch); currentPitch = pitch; }
	}

	// 目標音量を設定（即時 or 補間）
	void SetTargetVolume(float vol, bool immediate = false)
	{
		targetVolume = std::clamp(vol, 0.0f, 1.0f);
		if (immediate && instance) { instance->SetVolume(vol); currentVolume = vol; }
	}

	// 毎フレーム補間を進める。戻り値: インスタンスがまだ有効かどうか
	bool Update(float dt)
	{
		if (!instance) { return false; }

		// 停止済みになっていたら破棄
		if (instance->IsStopped()) { instance = nullptr; return false; }

		const float step = interpSpeed * dt;

		// ピッチ補間
		if (std::abs(currentPitch - targetPitch) > 0.001f)
		{
			currentPitch = std::lerp(currentPitch, targetPitch, std::min(step, 1.0f));
			instance->SetPitch(currentPitch);
		}

		// 音量補間
		if (std::abs(currentVolume - targetVolume) > 0.001f)
		{
			currentVolume = std::lerp(currentVolume, targetVolume, std::min(step, 1.0f));
			instance->SetVolume(currentVolume);
		}

		// 音量0で自動停止
		if (stopWhenSilent && currentVolume < 0.005f)
		{
			instance->Stop();
			instance = nullptr;
			return false;
		}

		return true;
	}

	void Stop()
	{
		if (instance) { instance->Stop(); instance = nullptr; }
	}
};


// ============================================================
// AudioManager
// ゲーム全体の音を統括管理するクラス（シングルトン）
//
// 扱う音の種類:
//   SE         … 一発鳴らして終わり（2D/3D）
//   LoopSound  … ループし続けるエンジン音など。ピッチ・音量を補間制御
//   BGM        … フェードイン/アウト/クロスフェード対応
// ============================================================
class AudioManager
{
public:
	static AudioManager& Instance()
	{
		static AudioManager instance;
		return instance;
	}

	void Release();
	void Init();
	void Update(float dt);

private:
	AudioManager() {};
	~AudioManager() { Release(); }

	// ----------------------------------------------------------
	// SE
	// ----------------------------------------------------------
	void PlaySE(const std::string& filePath, const Math::Vector3& pos, float volume = 1.0f);
	void PlaySE2D(const std::string& filePath, float volume = 1.0f);

	// ----------------------------------------------------------
	// LoopSound（エンジン音・アイドル音など）
	// ----------------------------------------------------------

	// ループ音を開始してハンドルを返す。既に再生中なら何もせず既存を返す
	LoopSoundHandle& StartLoopSound(LoopSoundHandle& handle,
		const std::string& filePath,
		float initVolume = 1.0f,
		float initPitch = 0.0f);

	// スピードレベルに応じてエンジン音を制御する
	void OnChangeSpeedLevel(int level);

	// ループサウンドの補間更新
	void UpdateLoopSounds(float dt);

	// エンジン駆動音（走行中）
	LoopSoundHandle m_engineDriveSound;

	// アイドル音（停止・低速時）
	LoopSoundHandle m_engineIdleSound;

	// スコアロール音（ループSE）
	LoopSoundHandle m_rollSound;

	// ----------------------------------------------------------
	// BGM
	// ----------------------------------------------------------
	void PlayBGMWithFadeIn(const std::string& filePath, float fadeDuration, bool loop = true);
	void StopBGMWithFadeOut(float fadeDuration);
	void UpdateBGMFade(float dt);

	std::shared_ptr<KdSoundInstance> m_currentBGM;
	std::shared_ptr<KdSoundInstance> m_nextBGM;

	enum class FadeState { None, FadeIn, FadeOut, CrossFade };
	FadeState m_fadeState = FadeState::None;
	float     m_fadeTimer = 0.0f;
	float     m_fadeDuration = 1.0f;

	std::string m_pendingBGMPath;
	float       m_pendingFadeDuration = 1.0f;
	bool        m_pendingBGMLoop = true;

	// ----------------------------------------------------------
	// イベント購読
	// ----------------------------------------------------------
	std::vector<ScopedSubscriber> m_subscriber;
};