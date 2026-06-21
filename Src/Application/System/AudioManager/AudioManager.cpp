#include "AudioManager.h"

// ============================================================
// 各スピードレベルに対応するエンジン音パラメータ定義
// ここを変えるだけで音の調整が完結する
// ============================================================
namespace EngineAudioParam
{
	struct Param
	{
		float drivePitch;   // 駆動音のピッチ（DirectXTK: -1〜1, 0=等速）
		float driveVolume;  // 駆動音の音量（0〜1）
		float idleVolume;   // アイドル音の音量（0〜1）
	};

	// インデックス = スピードレベル（0:停止, 1:低速, 2:中速, 3:高速 …）
	static const Param Table[] =
	{
		// drivePitch  driveVolume  idleVolume
		{  0.0f,       0.0f,        1.0f  },  // Lv0: 停止  → 駆動音消してアイドルのみ
		{  -0.3f,      0.4f,        0.6f  },  // Lv1: 低速  → 駆動音小さめ＋アイドル混在
		{  0.0f,       0.8f,        0.2f  },  // Lv2: 中速  → 駆動音メイン
		{  0.4f,       1.0f,        0.0f  },  // Lv3: 高速  → 駆動音フル、アイドル消える
		{  0.5f,       1.0f,        0.0f  },  // Lv4: 高速  → 駆動音フル、アイドル消える
		{  0.7f,       1.0f,        0.0f  },  // Lv5: 高速  → 駆動音フル、アイドル消える
		{  1.f,        1.0f,        0.0f  },  // Lv6: 高速  → 駆動音フル、アイドル消える
	};

	static constexpr int TableSize = static_cast<int>(std::size(Table));

	// ピッチの補間速度（毎秒どれだけ目標値に近づくか）
	static constexpr float PitchInterpSpeed = 3.0f;
	// 音量の補間速度
	static constexpr float VolumeInterpSpeed = 2.5f;
}

// ============================================================
// 解放
// ・ループ音・BGMを即時停止してインスタンスを手放す
// ・イベント購読を解除する
//　※ KdAudioManager より先に呼ばれることを想定
//    （シングルトンの破棄順に依存しないよう、明示的に Release() を
//      ゲーム終了処理から呼ぶことを推奨）
// ============================================================
void AudioManager::Release()
{
	// ループ音を即時停止（フェード待ちにしない）
	m_engineDriveSound.Stop();
	m_engineIdleSound.Stop();
	m_rollSound.Stop();

	// BGMを即時停止
	if (m_currentBGM) { m_currentBGM->Stop(); m_currentBGM = nullptr; }
	if (m_nextBGM) { m_nextBGM->Stop();    m_nextBGM = nullptr; }
	m_fadeState = FadeState::None;

	// イベント購読解除
	m_subscriber.clear();
}

// ============================================================
// 初期化：イベント購読の登録
// ============================================================
void AudioManager::Init()
{
	// ----- ヒット結果 → SE再生 -----
	m_subscriber.push_back(
		GLOBALEVENT.subscribe<Events::Player::HitResult>([this](const Events::Player::HitResult& e)
			{
				switch (e.type)
				{
				case Events::Player::HitResult::HitResultType::Bounced:
					if ((e.speedLevel - 3) < 0)return;
					PlaySE("Asset/Sound/Clash.wav", e.m_pos, 1.f + float(e.speedLevel - 3));
					break;
				case Events::Player::HitResult::HitResultType::Destroyed:
					PlaySE("Asset/Sound/Destroy.wav", e.m_pos, 0.85f);
					break;
				default:
					break;
				}
			})
	);

	// ----- スピードレベル変化 → エンジン音制御 -----
	m_subscriber.push_back(
		GLOBALEVENT.subscribe<Events::Player::ChangeSpeedLevel>([this](const Events::Player::ChangeSpeedLevel& e)
			{
				OnChangeSpeedLevel(e.level);  // e.level はイベント定義に合わせて変更
			})
	);

	// ----- タイトル開始 → BGMフェードイン -----
	m_subscriber.push_back(
		GLOBALEVENT.subscribe<Events::Else::TitleBegin>([this](const Events::Else::TitleBegin& e)
			{
				PlayBGMWithFadeIn("Asset/Sound/soul-drive.wav", 0.5f, true);
			})
	);

	m_subscriber.push_back(
		GLOBALEVENT.subscribe<Events::Else::TitleToGameBegin>([this](const Events::Else::TitleToGameBegin& e)
			{
				OnChangeSpeedLevel(0);
				PlaySE2D("Asset/Sound/Select.wav");
			})
	);

	m_subscriber.push_back(
		GLOBALEVENT.subscribe<Events::Player::GetSpeedUp>([this](const Events::Player::GetSpeedUp& e)
			{
				PlaySE("Asset/Sound/Item.wav", e.m_me.lock()->GetPos());
			})
	);

	m_subscriber.push_back(
		GLOBALEVENT.subscribe<Events::Player::DeliveryPointDeleted>([this](const Events::Player::DeliveryPointDeleted& e)
			{
				PlaySE("Asset/Sound/Beep.wav", e.m_me.lock()->GetPos());
			})
	);

	m_subscriber.push_back(
		GLOBALEVENT.subscribe<Events::Player::DeliveryPointCompleted>([this](const Events::Player::DeliveryPointCompleted& e)
			{
				PlaySE("Asset/Sound/Deliveryed.wav", e.m_me.lock()->GetPos());
			})
	);

	m_subscriber.push_back(
		GLOBALEVENT.subscribe<Events::Else::GameToResultBegin>([this](const Events::Else::GameToResultBegin& e)
			{
				StopBGMWithFadeOut(0.5f);
			})
	);

	m_subscriber.push_back(
		GLOBALEVENT.subscribe<Events::Else::ResultBegin>([this](const Events::Else::ResultBegin& e)
			{
				KdAudioManager::Instance().StopAllSound();
				PlayBGMWithFadeIn("Asset/Sound/Someday-in-the-Rain.wav", 0.5f, true);
			})
	);

	m_subscriber.push_back(
		GLOBALEVENT.subscribe<Events::Else::ResultEnd>([this](const Events::Else::ResultEnd& e)
			{
				PlaySE2D("Asset/Sound/Select.wav");
			})
	);

	m_subscriber.push_back(
		GLOBALEVENT.subscribe<Events::Else::DeliveryScoreRollBegin>([this](const Events::Else::DeliveryScoreRollBegin& e)
			{
				// ループ再生開始（既に鳴っていれば何もしない）
				StartLoopSound(m_rollSound, "Asset/Sound/Roll.wav", 1.0f, 0.0f);
			})
	);

	m_subscriber.push_back(
		GLOBALEVENT.subscribe<Events::Else::DestroyScoreRollEnd>([this](const Events::Else::DestroyScoreRollEnd& e)
			{
				m_rollSound.stopWhenSilent = true;
				m_rollSound.SetTargetVolume(0.0f);

				PlaySE2D("Asset/Sound/Fanfare.wav");
			})
	);



	// エンジン音・アイドル音の補間速度を設定
	m_engineDriveSound.interpSpeed = EngineAudioParam::PitchInterpSpeed;
	m_engineIdleSound.interpSpeed = EngineAudioParam::VolumeInterpSpeed;
}


// ============================================================
// 更新
// ============================================================
void AudioManager::Update(float dt)
{
	UpdateLoopSounds(dt);
	UpdateBGMFade(dt);
}


// ============================================================
// SE再生（3D）
// ============================================================
void AudioManager::PlaySE(const std::string& filePath, const Math::Vector3& pos, float volume)
{
	auto instance = KdAudioManager::Instance().Play3D(filePath, pos, false);
	instance->SetVolume(volume);
}

// ============================================================
// SE再生（2D）
// ============================================================
void AudioManager::PlaySE2D(const std::string& filePath, float volume)
{
	auto instance = KdAudioManager::Instance().Play(filePath, false);
	instance->SetVolume(volume);
}


// ============================================================
// ループ音の開始（既に再生中なら何もしない）
// ============================================================
LoopSoundHandle& AudioManager::StartLoopSound(LoopSoundHandle& handle,
	const std::string& filePath,
	float initVolume,
	float initPitch)
{
	// 既に有効なインスタンスがあれば再生継続
	if (handle.IsValid()) { return handle; }

	auto inst = KdAudioManager::Instance().Play(filePath, true);  // loop=true
	if (inst)
	{
		inst->SetVolume(initVolume);
		inst->SetPitch(initPitch);
	}

	handle.instance = inst;
	handle.currentVolume = initVolume;
	handle.targetVolume = initVolume;
	handle.currentPitch = initPitch;
	handle.targetPitch = initPitch;
	handle.stopWhenSilent = false;

	return handle;
}


// ============================================================
// スピードレベル変化時のエンジン音制御
//
// 考え方:
//   駆動音・アイドル音はどちらも常に再生し続け、
//   音量とピッチをレベルに応じて補間で変化させる。
//   音量が0になってもインスタンスは止めない（= 即座に鳴り直せる）。
//   ただし完全なアイドル状態(Lv0)では駆動音は停止まで持っていく。
// ============================================================
void AudioManager::OnChangeSpeedLevel(int level)
{
	// テーブル範囲外ガード
	const int clampedLevel = std::clamp(level, 0, EngineAudioParam::TableSize - 1);
	const auto& param = EngineAudioParam::Table[clampedLevel];

	// ----- 駆動音 -----
	if (param.driveVolume > 0.0f)
	{
		// 駆動音が必要 → 未再生なら開始（音量0・同ピッチで始めてフェードイン）
		StartLoopSound(m_engineDriveSound, "Asset/Sound/Engine_Loop.wav", 0.0f, param.drivePitch);

		m_engineDriveSound.stopWhenSilent = false;
		m_engineDriveSound.SetTargetPitch(param.drivePitch);
		m_engineDriveSound.SetTargetVolume(param.driveVolume);
	}
	else
	{
		// 駆動音が不要 → フェードアウト後に自動停止
		if (m_engineDriveSound.IsValid())
		{
			m_engineDriveSound.stopWhenSilent = true;
			m_engineDriveSound.SetTargetVolume(0.0f);
			// ピッチは現在値のまま（止まるまでキープ）
		}
	}

	// ----- アイドル音 -----
	if (param.idleVolume > 0.0f)
	{
		// アイドル音が必要 → 未再生なら開始
		StartLoopSound(m_engineIdleSound, "Asset/Sound/Idle.wav", 0.0f, 0.0f);

		m_engineIdleSound.stopWhenSilent = false;
		m_engineIdleSound.SetTargetVolume(param.idleVolume);
	}
	else
	{
		// アイドル音が不要 → フェードアウト後に自動停止
		if (m_engineIdleSound.IsValid())
		{
			m_engineIdleSound.stopWhenSilent = true;
			m_engineIdleSound.SetTargetVolume(0.0f);
		}
	}
}


// ============================================================
// ループサウンドの補間更新
// ============================================================
void AudioManager::UpdateLoopSounds(float dt)
{
	m_engineDriveSound.Update(dt);
	m_engineIdleSound.Update(dt);
	m_rollSound.Update(dt);
}


// ============================================================
// BGM：フェードイン再生
// ============================================================
void AudioManager::PlayBGMWithFadeIn(const std::string& filePath, float fadeDuration, bool loop)
{
	m_fadeDuration = fadeDuration;
	m_fadeTimer = 0.0f;

	m_nextBGM = KdAudioManager::Instance().Play(filePath, loop);
	if (m_nextBGM) { m_nextBGM->SetVolume(0.0f); }

	if (m_currentBGM && m_currentBGM->IsPlaying())
	{
		m_fadeState = FadeState::CrossFade;
	}
	else
	{
		m_currentBGM = m_nextBGM;
		m_nextBGM = nullptr;
		m_fadeState = FadeState::FadeIn;
	}
}

// ============================================================
// BGM：フェードアウト停止
// ============================================================
void AudioManager::StopBGMWithFadeOut(float fadeDuration)
{
	if (!m_currentBGM || !m_currentBGM->IsPlaying()) { return; }

	m_fadeDuration = fadeDuration;
	m_fadeTimer = 0.0f;
	m_fadeState = FadeState::FadeOut;
}

// ============================================================
// BGMフェード更新
// ============================================================
void AudioManager::UpdateBGMFade(float dt)
{
	if (m_fadeState == FadeState::None) { return; }

	m_fadeTimer += dt;
	const float progress = (m_fadeDuration > 0.0f)
		? std::min(m_fadeTimer / m_fadeDuration, 1.0f)
		: 1.0f;

	switch (m_fadeState)
	{
	case FadeState::FadeIn:
		if (m_currentBGM) { m_currentBGM->SetVolume(progress); }
		if (progress >= 1.0f) { m_fadeState = FadeState::None; }
		break;

	case FadeState::FadeOut:
		if (m_currentBGM) { m_currentBGM->SetVolume(1.0f - progress); }
		if (progress >= 1.0f)
		{
			if (m_currentBGM) { m_currentBGM->Stop(); }
			m_currentBGM = nullptr;

			if (!m_pendingBGMPath.empty())
			{
				PlayBGMWithFadeIn(m_pendingBGMPath, m_pendingFadeDuration, m_pendingBGMLoop);
				m_pendingBGMPath.clear();
			}
			else
			{
				m_fadeState = FadeState::None;
			}
		}
		break;

	case FadeState::CrossFade:
		if (m_currentBGM) { m_currentBGM->SetVolume(1.0f - progress); }
		if (m_nextBGM) { m_nextBGM->SetVolume(progress); }
		if (progress >= 1.0f)
		{
			if (m_currentBGM) { m_currentBGM->Stop(); }
			m_currentBGM = m_nextBGM;
			m_nextBGM = nullptr;
			m_fadeState = FadeState::None;
		}
		break;

	default:
		break;
	}
}