#include "CameraManager.h"

#include "Application/Object/Character/Player/Player.h"
#include "../Reader/Reader.h"
#include "../ConvertScreen/ConvertScreen.h"

void CameraManager::Init()
{
	m_state = CameraState::Title;
	//m_state = CameraState::Game;
	SetUp(m_state);

	m_camera = std::make_unique<KdCamera>();
	m_camera->SetProjectionMatrix(m_projection,2000.f,0.05f);
	
	m_blurIntensity = 0.f;
	m_blurMaxRange = 0.f;
	m_blurMinRange = 0.f;

	KdShaderManager::Instance().WorkAmbientController().SetFogEnable(false, true);
	KdShaderManager::Instance().WorkAmbientController().SetheightFog({ 1,1,1 }, 0, 2, 0);

	m_speedSub = GLOBALEVENT.subscribe<Events::Player::ChangeSpeedLevel>([this](const Events::Player::ChangeSpeedLevel& e)
		{
			if (e.level != (int)SpeedLevel::Clash)
			{
				m_targetProj = 60.f + e.level * 12.f;
				if (e.level >= (int)SpeedLevel::Speed1)
				{
					m_blurIntensity = 0.3f + e.level * 0.3f;
					m_blurMinRange = 0.5f - e.level * 0.06f;
					m_blurMaxRange = 1.f -e.level * 0.05f;
				}
				else
				{
					m_blurIntensity = 0.f;
					m_blurMaxRange = 0.f;
					m_blurMinRange = 0.f;
				}
			}
			else
			{
				m_targetProj = 60.f;
				m_blurIntensity = 0.f;
				m_blurMaxRange = 0.f;
				m_blurMinRange = 0.f;
			}
		});

	m_hitSub = GLOBALEVENT.subscribe<Events::Player::HitResult>([this](const Events::Player::HitResult& e)
		{
			switch (e.type)
			{
			case Events::Player::HitResult::HitResultType::Destroyed:
				// シェイク開始（例：強さ0.025f, 時間0.3秒）
				m_shakeStrength = 0.025f;
				m_shakeTime = 0.3f;
				return;
			case Events::Player::HitResult::HitResultType::Bounced:
				if (e.speedLevel <= 3 || e.speedLevel == 7)return;
				m_shakeStrength = 0.025f + 0.025f * (e.speedLevel - 3);
				m_shakeTime = 0.3f;
				return;
			default:
				return;
			}
		});

	m_toGameSub = GLOBALEVENT.subscribe<Events::Else::TitleToGameBegin>([this](const Events::Else::TitleToGameBegin& e)
		{
			SetUp(CameraState::TitletoGame);
		});
}

void CameraManager::SetUp(CameraState state)
{
	m_state = state;
	switch (state)
	{
	case CameraState::Title:
	{
		m_camDis = { -0.01f,0.055f,-0.1f };
		m_camAng.x = 0.f;
		m_camAng.y = 270.f;
		m_projection = 60.0f;
		m_ambientLight = 1.f;
		KdShaderManager::Instance().WorkAmbientController().SetAmbientLight(Math::Vector4(1.f, 1.f, 1.f, m_ambientLight));
		return;
	}
	case CameraState::TitletoGame:
		m_transitionProgress = 0.0f;
		m_startDis = m_camDis;
		m_startAng = m_camAng;
		m_startProj = m_projection;
		return;
	case CameraState::Game:
		m_camDis = DEF_DIS;
		m_camAng.x = 25.0f;
		m_projection = 60.0f;
		m_targetAngle = -20.f;
		m_targetProj = m_projection;
		m_targetPos = m_camDis;
		m_speed = 3.0f;
		m_blurIntensity = 0.f;
		m_blurMaxRange = 0.f;
		m_blurMinRange = 0.f;
		KdShaderManager::Instance().m_postProcessShader.SetSpeedBlurSamplingNum(8);       // サンプリング数(品質)
		return;
	default:
		return;
	}
}

void CameraManager::Update(float dt)
{
	switch (m_state)
	{
	case CameraState::Title:
		UpdateTitle(dt);
		break;
	case CameraState::TitletoGame:
		UpdateTitletoGame(dt);
		break;
	case CameraState::Game:
		UpdateGame(dt);
		break;
	default:
		break;
	}
	m_camera->SetToShader();

	CameraData data;
	data.mat = m_camera->GetCameraMatrix();
	Reader::Instance().WriteCamera(data);
}

void CameraManager::UpdateTitle(float dt)
{
	std::shared_ptr<Player> _targetObj = nullptr;
	if (!m_targetObj.expired()) _targetObj = m_targetObj.lock();

	Math::Matrix targetMat = _targetObj->GetMatrix();

	// シェイクの減衰処理
	Math::Vector3 shakeOffset = Math::Vector3::Zero;
	if (m_shakeTime > 0.0f)
	{
		m_shakeTime -= dt;
		// ランダムな方向に揺らす
		shakeOffset.x = KdRandom::GetFloat(-1.f, 1.f) * m_shakeStrength;
		shakeOffset.y = KdRandom::GetFloat(-1.f, 1.f) * m_shakeStrength;

		// 時間経過とともに弱める
		m_shakeStrength *= 0.9f;
	}

	m_camPos = targetMat.Translation();

	Math::Matrix mat =
		Math::Matrix::CreateRotationX(DirectX::XMConvertToRadians(m_camAng.x)) *
		Math::Matrix::CreateTranslation(m_camDis + shakeOffset) *
		// ここで (カメラの基本Y回転 + ステアリングによるYオフセット) を計算
		Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_camAng.y)) *
		targetMat;

	m_camera->SetCameraMatrix(mat);	
}

void CameraManager::UpdateTitletoGame(float dt)
{
	std::shared_ptr<Player> _targetObj = nullptr;
	if (!m_targetObj.expired()) _targetObj = m_targetObj.lock();
	if (!_targetObj) return;

	// 1. 進行度の更新 (0.0 -> 1.0)
	m_transitionProgress += dt / TRANSITION_TIME;
	if (m_transitionProgress > 1.0f)
	{
		m_transitionProgress = 1.0f;
	}

	// 2. イージングの計算（EaseInOutCubic）
	// 直線的な変化(Lerp)ではなく、最初はゆっくり、途中で加速し、最後はゆっくり止まる滑らかな動きにします
	float t = m_transitionProgress;
	float easeT = t < 0.5f ? 4.0f * t * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;

	// 3. 目標値の設定（Gameステートの設定値に合わせる）
	Math::Vector3 targetDis = DEF_DIS;
	float targetAngX = 25.0f;

	// TitleのY角度(270度)から「360度 + 90度 = 450度」回転させて正面(720度)に向かせる
	float targetAngY = 720.0f;
	float targetProj = 60.0f;

	// 4. 各パラメータの補間
	m_camDis = Math::Vector3::Lerp(m_startDis, targetDis, easeT);
	m_camAng.x = std::lerp(m_startAng.x, targetAngX, easeT);
	m_camAng.y = std::lerp(m_startAng.y, targetAngY, easeT);

	if (m_projection != targetProj)
	{
		m_projection = std::lerp(m_startProj, targetProj, easeT);
		m_camera->SetProjectionMatrix(m_projection, 2000.f, 0.05f);
	}

	// 5. カメラ行列の計算と適用（Titleと同様にターゲット基準）
	Math::Matrix targetMat = _targetObj->GetMatrix();
	m_camPos = targetMat.Translation();

	Math::Matrix mat =
		Math::Matrix::CreateRotationX(DirectX::XMConvertToRadians(m_camAng.x)) *
		Math::Matrix::CreateTranslation(m_camDis) *
		Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_camAng.y)) *
		targetMat;

	m_camera->SetCameraMatrix(mat);

	float targetLight = 0.75f;
	m_ambientLight = std::lerp(m_ambientLight, targetLight, easeT);

	KdShaderManager::Instance().WorkAmbientController().SetAmbientLight(Math::Vector4(1.f, 1.f, 1.f, m_ambientLight));

	// 6. 遷移完了判定
	if (m_transitionProgress >= 1.0f)
	{
		// Gameステートへ移行
		SetUp(CameraState::Game);
		GLOBALEVENT.publish(Events::Else::TitleToGameEnd());
		GLOBALEVENT.publish(Events::Else::GameStart());
	}
}

void CameraManager::UpdateGame(float dt)
{
	std::shared_ptr<Player> _targetObj = nullptr;
	if (!m_targetObj.expired()) _targetObj = m_targetObj.lock();

	UpdateProjection(dt);
	UpdateAngle(_targetObj, dt);
	UpdateDistance(_targetObj, dt);

	Math::Matrix targetMat = _targetObj->GetMatrix();
	m_camPos.y = std::lerp(m_camPos.y, targetMat.Translation().y, m_speed * 2.f * dt);
	m_camPos.x = targetMat.Translation().x;
	m_camPos.z = targetMat.Translation().z;
	//targetMat.Translation(m_camPos);

	// スケール・回転・位置を分解する
	Math::Vector3 scale, translation;
	Math::Quaternion rotation;
	targetMat.Decompose(scale, rotation, translation);

	// スケールなしで行列を再構築
	Math::Matrix _mat = Math::Matrix::CreateFromQuaternion(rotation)
		* Math::Matrix::CreateTranslation(m_camPos);

	// シェイクの減衰処理
	Math::Vector3 shakeOffset = Math::Vector3::Zero;
	if (m_shakeTime > 0.0f)
	{
		m_shakeTime -= dt;
		// ランダムな方向に揺らす
		shakeOffset.x = KdRandom::GetFloat(-1.f, 1.f) * m_shakeStrength;
		shakeOffset.y = KdRandom::GetFloat(-1.f, 1.f) * m_shakeStrength;

		// 時間経過とともに弱める
		m_shakeStrength *= 0.9f;
	}

	// カメラの姿勢を計算
	// 「車自体の向き(targetMat)」に対し、「カメラの基本角度(m_camAng)」と「オフセット(currentSteerOffset)」を加える
	Math::Matrix mat =
		Math::Matrix::CreateRotationX(DirectX::XMConvertToRadians(m_camAng.x)) *
		Math::Matrix::CreateTranslation(m_camDis + shakeOffset) *
		// ここで (カメラの基本Y回転 + ステアリングによるYオフセット) を計算
		Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_camAng.y + m_steeringOffset)) *
		//targetMat;
		_mat;

	m_camera->SetCameraMatrix(mat);

	KdShaderManager::Instance().m_postProcessShader.SetSpeedBlurIntensity(m_blurIntensity);      // 0.0~1.0で強度を直接指定
	KdShaderManager::Instance().m_postProcessShader.SetSpeedBlurRange(m_blurMinRange, m_blurMaxRange);    // ブラーがかかり始める/最大になる範囲
}

void CameraManager::DrawSprite()
{
	auto sprites = ConvertScreen::Instance().AcceptConvertScreen();
	Math::Vector3 screenPos;

	for (auto& sprite : sprites)
	{
		Math::Vector3 screenPos;

		// カメラ後ろ判定のみ自前で行う
		Math::Matrix vp = m_camera->GetCameraViewMatrix() * m_camera->GetProjMatrix();
		Math::Vector4 clip = Math::Vector4::Transform(
			Math::Vector4(
				sprite.mat.Translation().x,
				sprite.mat.Translation().y,
				sprite.mat.Translation().z,
				1.0f), vp);

		// カメラ後ろはスキップ
		if (clip.w <= 0.0f) continue;

		// スクリーン座標変換はそのまま既存処理に任せる
		m_camera->ConvertWorldToScreenDetail(sprite.mat.Translation(), screenPos);
		KdShaderManager::Instance().m_spriteShader.DrawTex(
			sprite.tex,
			static_cast<int>(screenPos.x),
			static_cast<int>(screenPos.y));
	}
}

void CameraManager::UpdateProjection(float dt)
{
	if (m_projection == m_targetProj)return;
	float t = m_speed * dt;
	m_projection = std::lerp(m_projection, m_targetProj, t);
	m_camera->SetProjectionMatrix(m_projection, 2000.f, 0.05f);
}

void CameraManager::UpdateAngle(const std::shared_ptr<Player>& target, float dt)
{
	float steerInput = target->GetSteeringInput();
	float targetOffset = steerInput * m_targetAngle; // 目標角度

	// 補間
	float t = m_speed * dt;
	m_steeringOffset = std::lerp(m_steeringOffset, targetOffset, t);

}

void CameraManager::UpdateDistance(const std::shared_ptr<Player>&target, float dt)
{
	float t = m_speed * dt;
	m_camDis = Math::Vector3::Lerp(m_camDis, m_targetPos, t);
}
