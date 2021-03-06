#include "stdafx.h"
#include "AICar.h"
#include "ModelRender.h"
#include "Level3D.h"
#include "GameTime.h"
#include "GameMainState.h"
#include "BezierCurve.h"
#include "PlayerConstData.h"
#include "SoundCue.h"
#include "UIConstData.h"

namespace nsNinjaAttract
{
	/**
	 * @brief AIキャラクター関連のネームスペース
	*/
	namespace nsAICharacter
	{
		int CAICar::m_carTotalNumber = 0;

		// AIキャラクターの定数データを使用可能にする
		using namespace nsAICharacterConstData;

		/**
		 * @brief コンストラクタ
		*/
		CAICar::CAICar()
		{
			// 生成した瞬間に呼んでほしい処理

			// モデルレンダラーの生成
			m_modelRender = NewGO<nsGraphic::nsModel::CModelRender>(nsCommonData::enPriorityFirst);
			// ゲームステートに車を登録
			nsGameState::CGameMainState::GetInstance()->AddAICar(this);
		}

		/**
		 * @brief Updateの直前で呼ばれる開始処理
		 * @return アップデートを行うか？
		*/
		bool CAICar::Start()
		{
			// 車の種類の番号。若い番号から順に作られる。
			m_carNumber = m_carTotalNumber % enCarTypeNum;
			m_carTotalNumber++;

			// モデルの初期化
			m_modelRender->Init(kCarModelFilePath[m_carNumber]);
			// LODの設定
			m_modelRender->InitLODModel(kCarModelLODFilePath[m_carNumber]);
			m_modelRender->SetIsEnableLOD(true);
			m_modelRender->SetDistanceLOD(500.0f);


			// 車の移動速度を設定
			m_moveSpeed = kCarSpeed;

			// 乱数の初期化
			InitRand();

			// エフェクトの初期化
			InitEffect();

			// サウンドの初期化
			InitSound();

			return true;
		}

		/**
		 * @brief 消去される時に呼ばれる処理
		*/
		void CAICar::OnDestroy()
		{
			DeleteGO(m_modelRender);

			DeleteGO(m_explotionEF);
			for (auto& smokeEF : m_smokeEFs)
			{
				DeleteGO(smokeEF);
			}
			DeleteGO(m_smokeExplotionEF);

			DeleteGO(m_driveSC);
			DeleteGO(m_driftSC);
			DeleteGO(m_fireExplosionSC);
			DeleteGO(m_carExplosionSC);

			return;
		}

		/**
		 * @brief 更新処理
		*/
		void CAICar::Update()
		{
			if (m_isCaptured)
			{
				// 捕まっている時の更新
				// この後の更新処理も行うか？
				const bool doUpdate = UpdateOnIsCaptured();

				if (doUpdate != true)
				{
					// 行わない。早期リターン。
					return;
				}

			}

			if (m_isMoveEnd)
			{
				// 移動終了

				// 次に進むウェイポイントを探す
				FindNextWayPoint();
			}

			// 移動中

			// ウェイポイント上を移動する
			MoveOnWayPoint();

			// 回転を更新
			UpdateRotating();

			// サウンドの更新
			UpdateSound();


			return;
		}

		/**
		 * @brief 初期化処理
		 * @param[in] AICharaInitData AIキャラクターの初期化データ
		 * @param[in] pos 座標
		 * @param[in] rot 回転
		 * @param[in] scale 拡大
		*/
		void CAICar::Init(
			const nsAI::SAICharacterInitData& AICharaInitData,
			const Vector3& pos,
			const Quaternion rot,
			const Vector3& scale
		)
		{
			// ウェイポイントの参照を得る
			m_wayPointsRef = AICharaInitData.naviMeshTargetPointsRef;

			m_modelRender->SetPosition(pos);
			m_modelRender->SetRotation(rot);
			m_modelRender->SetScale(scale);

			return;
		}

		/**
		 * @brief 乱数の初期化
		*/
		void CAICar::InitRand()
		{
			// 非決定的な乱数生成器でシード生成機を生成
			std::random_device rnd;
			// メルセンヌツイスターの32ビット版、引数は初期シード
			m_mt = std::make_unique<std::mt19937>(rnd());
			// 範囲の一様乱数
			m_rand = std::make_unique<std::uniform_int_distribution<>>(0, enCandidateTargetPointTypeNum - 1);

			return;
		}


		/**
		 * @brief エフェクトの初期化
		*/
		void CAICar::InitEffect()
		{
			// 爆発エフェクトの生成と初期化
			m_explotionEF = NewGO<Effect>(nsCommonData::enPrioritySecond);
			m_explotionEF->Init(kExplotionEffectFilePath);

			// 煙のエフェクトの生成と初期化
			for (auto& smokeEF : m_smokeEFs)
			{
				smokeEF = NewGO<Effect>(nsCommonData::enPrioritySecond);
				smokeEF->Init(kSmokeEffectFilePath);
			}

			// 爆発時の煙のエフェクトの生成と初期化
			m_smokeExplotionEF = NewGO<Effect>(nsCommonData::enPrioritySecond);
			m_smokeExplotionEF->Init(kSmokeExplosionEffectFilePath);

			return;
		}

		/**
		 * @brief サウンドの初期化
		*/
		void CAICar::InitSound()
		{			
			m_driveSC = NewGO<nsSound::CSoundCue>(nsCommonData::enPriorityFirst);
			m_driveSC->Init(kCarDriveSoundFilePath, nsSound::CSoundCue::enSE);

			m_driftSC = NewGO<nsSound::CSoundCue>(nsCommonData::enPriorityFirst);
			m_driftSC->Init(kCarDriftSoundFilePath, nsSound::CSoundCue::enSE);

			m_fireExplosionSC = NewGO<nsSound::CSoundCue>(nsCommonData::enPriorityFirst);
			m_fireExplosionSC->Init(kFireExplosionSoundFilePath, nsSound::CSoundCue::enSE);

			m_carExplosionSC = NewGO<nsSound::CSoundCue>(nsCommonData::enPriorityFirst);
			m_carExplosionSC->Init(kCarExplosionSoundFilePath, nsSound::CSoundCue::enSE);

			m_driveTimer = kDriveSoundTime;

			return;
		}

		/**
		 * @brief 捕まっている時の更新
		 * @return この後も更新処理を行うか？
		*/
		bool CAICar::UpdateOnIsCaptured()
		{
			if (m_isCaptureTimer < kSmokeTime)
			{
				// タイマーが煙の時間より小さいとき

				// 煙のエフェクトの更新
				UpdateSmokeEffect();

				if (m_ninjyutuEFRef)
				{
					// 忍術のエフェクトの参照が渡されているなら、座標と回転を更新する。
					Vector3 pos = m_modelRender->GetPosition();
					pos.y += nsPlayer::nsPlayerConstData::nsCatchEnemyConstData::kNinjyutuEffectPosBufHeight;
					m_ninjyutuEFRef->SetPosition(pos);
					m_ninjyutuEFRef->SetRotation(m_modelRender->GetRotation());
				}

				// タイマーを進める
				m_isCaptureTimer += nsTimer::GameTime().GetFrameDeltaTime();

				// まだ動いていてほしいので、trueを戻す。
				return true;
			}

			// タイマーが煙の時間より大きいとき

			if (m_modelRender->IsActive() != true)
			{
				// モデルが非表示なら、何もしない。
				// もう動いてほしくないので、falseを戻す。
				return false;
			}

			// モデルがまだ表示されていたら、エフェクトを再生して、モデルを非表示にする。

			// エフェクトの座標
			Vector3 pos = m_modelRender->GetPosition();
			// 自身の位置から、少し上にあげる。
			pos.y += kExplotionPosBufHeight;
			// 爆発エフェクトの座標と回転と拡大率を設定
			m_explotionEF->SetPosition(pos);
			m_explotionEF->SetRotation(m_modelRender->GetRotation());
			m_explotionEF->SetScale(kExplotionScale);
			// 爆発エフェクトの再生
			m_explotionEF->Play();

			// エフェクトの回転
			Quaternion qRot;
			// 横に煙を出すエフェクトのため、回転させて立てる。
			qRot.SetRotationDegX(90.0f);
			// モデルの回転も考慮する
			qRot.Multiply(m_modelRender->GetRotation());
			// 爆発時の煙のエフェクトの座標と回転を設定する
			m_smokeExplotionEF->SetPosition(m_modelRender->GetPosition());
			m_smokeExplotionEF->SetRotation(qRot);
			// 爆発時の煙のエフェクトを再生する
			m_smokeExplotionEF->Play();

			// モデルを非表示にする
			m_modelRender->Deactivate();

			// もういらないので、渡された参照をリセットする。
			m_ninjyutuEFRef = nullptr;

			const float volume = CalcSoundVolume();
			m_fireExplosionSC->SetVolume(volume * kFireExplosionSoundVolume);
			m_carExplosionSC->SetVolume(volume * kCarExplosionSoundVolume);
			m_fireExplosionSC->Play(false);
			m_carExplosionSC->Play(false);

			// サウンドを停止
			StopSound();

			// クリアしたミッションの種類
			int clearMissionType = m_carNumber % (nsUI::nsMissionUIConstData::enMT_carBlue  + 1);
			// ステートにミッションクリアを通知
			nsGameState::CGameMainState::GetInstance()->ClearOneMission(
				static_cast<nsUI::nsMissionUIConstData::EnMissionType>(clearMissionType)
			);

			// もう動いてほしくないので、falseを戻す。
			return false;

		}


		/**
		 * @brief 煙のエフェクトの更新
		*/
		void CAICar::UpdateSmokeEffect()
		{
			// 煙の時間の進み率
			float rate = m_isCaptureTimer / kSmokeTime;
			// 煙の時間の進み率から、出す煙エフェクトの要素番号を決める
			int index = static_cast<int>(static_cast<float>(kSmokeNum) * rate);
			if (index > kSmokeNum)
			{
				// もし、煙の数より多い要素番号になったら、最大数にしておく。
				index = kSmokeNum - 1;
			}

			if (m_smokeEFs[index]->IsPlay() == true)
			{
				// この要素番号の煙エフェクトが、再生されていたら、何もしない。
				// 早期リターン
				return;
			}

			// この要素番号の煙エフェクトが、まだ再生されていなかったら、エフェクトを再生する。

			// エフェクトの座標
			Vector3 pos = m_modelRender->GetPosition();
			// ちょっと前側から出す
			Vector3 carForward = Vector3::Front;
			m_modelRender->GetRotation().Apply(carForward);
			carForward.Scale(kSmokePosBufForward);
			pos += carForward;
			// ちょっと上から出す
			pos.y += kSmokePosBufHeight;

			// エフェクトの回転
			Quaternion qRot;
			// 横に煙を出すエフェクトだから、回転させて立てる。
			qRot.SetRotationDegX(90.0f);
			// モデルの回転も考慮する
			qRot.Multiply(m_modelRender->GetRotation());


			// 座標と回転を設定
			m_smokeEFs[index]->SetPosition(pos);
			m_smokeEFs[index]->SetRotation(qRot);
			// エフェクトを再生する
			m_smokeEFs[index]->Play();

			return;
		}

		/**
		 * @brief 次のウェイポイントを探す
		*/
		void CAICar::FindNextWayPoint()
		{
			// ターゲットポイントの候補
			std::unordered_map<EnCandidateTargetPointType, Vector3> candidateTargetPoints;
			// 自身からターゲットポイントの候補への距離
			float posToCandidateTPLen[enCandidateTargetPointTypeNum] = {};
			for (int candidateTPType = 0; candidateTPType < enCandidateTargetPointTypeNum; candidateTPType++)
			{
				switch (candidateTPType)
				{
				case enTurnRight:
					posToCandidateTPLen[candidateTPType] = FLT_MIN;
					break;
				case enTurnLeft:
				case enGoStraight:
					posToCandidateTPLen[candidateTPType] = FLT_MAX;
					break;
				}
			}

			// ターゲットポイントの中から次の移動対象のターゲットポイントを探し出す。
			for (const auto& targetPos : *m_wayPointsRef)
			{
				// 除外するか？
				bool isExclude = false;

				// 自身の座標からターゲット座標へのベクトル
				const Vector3 posToTargetPosVec = targetPos - m_modelRender->GetPosition();
				// 自身の座標からターゲット座標への距離
				const float posToTargetPosLen = posToTargetPosVec.Length();
				// 移動方向とターゲット座標への方向のラジアン角
				float moveToPTTAngleRad = 0.0f;

				//////// 1.一定以上の距離があるターゲット座標を除外する ////////

				// 距離によるターゲットポイントの除外
				isExclude = ExcludeForDistance(posToTargetPosLen);

				// 除外
				if (isExclude)
				{
					continue;
				}

				//////// 2. 一定以上の角度より上のターゲットは除外 ////////

				// 自身の座標からターゲット座標への方向ベクトル
				Vector3 posToTargetPosDir = posToTargetPosVec;
				posToTargetPosDir.Normalize();

				// 角度によるターゲットポイントの除外
				isExclude = ExcludeForAngle(&moveToPTTAngleRad, posToTargetPosDir);

				// 除外
				if (isExclude)
				{
					continue;
				}

				//////// 3.候補に入れるターゲットがあれば候補に入れる ////////

				BeCandidateTargetPoint(
					&candidateTargetPoints,
					targetPos,
					posToCandidateTPLen,
					posToTargetPosLen,
					posToTargetPosDir
				);
			}

			// 候補が見つからなかった場合は何もしない
			if (candidateTargetPoints.empty())
			{
				return;
			}

			// ターゲットポイントを探せたか？
			bool IsFindedTargetPoint = false;
			EnCandidateTargetPointType randCandidateTPType = enTurnRight;

			while (IsFindedTargetPoint == false)
			{
				// ランダムなターゲットポイントの番号
				int randTargetNum = (*m_rand)(*m_mt);
				randCandidateTPType = static_cast<EnCandidateTargetPointType>(randTargetNum);

				// あったらターゲットポイントを探せた
				if (candidateTargetPoints.count(randCandidateTPType))
				{
					IsFindedTargetPoint = true;
					break;
				}
			}

			// 移動終了フラグをリセットする
			m_isMoveEnd = false;

			// パスを構築
			PathBuilding(randCandidateTPType, candidateTargetPoints[randCandidateTPType]);

			return;
		}

		/**
		 * @brief ウェイポイント上を移動する
		*/
		void CAICar::MoveOnWayPoint()
		{
			// 移動前の座標
			const Vector3 posBeforeMove = m_modelRender->GetPosition();

			// パス移動後の座標
			// パス移動
			const Vector3 posAfterMove = m_path.Move(
				posBeforeMove,
				m_moveSpeed * nsTimer::CGameTime().GetFrameDeltaTime(),
				m_isMoveEnd,
				PhysicsWorld::GetInstance()
			);

			// 移動方向を計算
			m_moveDir = posAfterMove - posBeforeMove;;
			m_moveDir.Normalize();

			// モデルの座標を更新
			m_modelRender->SetPosition(posAfterMove);

		}


		/**
		 * @brief 回転を更新
		*/
		void CAICar::UpdateRotating()
		{
			// 回転させる向きベクトル
			Vector3 rotateVec = Vector3::Front;
			// 現在のセクションのタグ
			nsAI::CPath::EnSectionTag sectionTag = m_path.GetCurrentSectionTag();
			m_moveSpeed = kCarSpeed;

			if (sectionTag == nsAI::CPath::enLeftCurveStart ||
				sectionTag == nsAI::CPath::enRightCurveStart)
			{
				// セクションが、カーブの開始地点、または、カーブの終了地点

				// 現在の移動の方向を軸に計算する
				rotateVec = m_moveDir;
				// ドリフトの回転
				Quaternion driftRot = Quaternion::Identity;
				// セクションの開始座標から現在の座標へのベクトル
				const Vector3 fromSectionStartPos =
					m_modelRender->GetPosition() - m_path.GetCurrentSection().startPos;
				// 現在、セクションの何％の位置にいるかを、ドリフトの回転率とする。
				float driftRate = fromSectionStartPos.Length() / m_path.GetCurrentSection().length;
				// 最小ドリフトの角度
				float minDriftAngle = 0.0f;
				// 最大ドリフトの角度
				float maxDriftAngle = kCarDriftMaxAngle;
				// カーブ終了セクションなら、最小と最大を反転する
				if (sectionTag == nsAI::CPath::enLeftCurveEnd || sectionTag == nsAI::CPath::enRightCurveEnd)
				{
					minDriftAngle = kCarDriftMaxAngle;
					maxDriftAngle = 0.0f;
				}
				// ドリフト率に応じて、ドリフトの角度を線形補完する。
				float driftAngle = Math::Lerp<float>(min(driftRate, 1.0f), minDriftAngle, maxDriftAngle);
				
				// 左折なら、角度を反転させる。
				if (sectionTag == nsAI::CPath::enLeftCurveStart || sectionTag == nsAI::CPath::enLeftCurveEnd)
				{
					driftAngle = -driftAngle;
				}
				driftRot.SetRotationDegY(driftAngle);
				driftRot.Apply(rotateVec);
			}
			else if (sectionTag == nsAI::CPath::enLeftCurve ||
				sectionTag == nsAI::CPath::enRightCurve)
			{
				// セクションが、カーブ中のとき

				// カーブの中心座標から、現在の座標への方向ベクトル。
				Vector3 fromCurveCenterPosNorm = m_modelRender->GetPosition() - m_curveCenterPosition;
				fromCurveCenterPosNorm.Normalize();
				// 移動方向だと、回転にカクツキができてしまうため、中心からのベクトルを軸に計算する。
				rotateVec = fromCurveCenterPosNorm;

				// カーブの中心からの角度率
				float angleRate = Dot(fromCurveCenterPosNorm, m_fromCurveCenterToCurveStartVec);
				angleRate = acosf(angleRate);
				// 0度〜90度が、0.0f〜1.0fになる。
				angleRate = angleRate / (3.14f * 0.5f);
				// ドリフトに加える回転
				float addAngle = 0.0f;

				// ドリフトの回転をもとに戻す
				if (angleRate >= kCarDriftTurnBackRate)
				{
					// 補間率。0.0f〜(1.0f-kCarDriftTurnBackRate)の値を、0.0f〜1.0fの値に直す。
					float t = (angleRate - kCarDriftTurnBackRate) / kCarDriftTurnBackRate;
					addAngle = Math::Lerp<float>(t, 0.0f, kCarDriftMaxAngle);
				}

				// ドリフト中のスピードダウン
				if (angleRate <= kCarDriftSpeedDownRate)
				{
					// 指数関数的に変化にする。
					float t = 1.0f - (angleRate / kCarDriftSpeedDownRate);
					t = 1.0f - pow(t, 2.0f);
					m_moveSpeed = Math::Lerp<float>(min(t,1.0f),  kCarSpeed, kCarSpeed * kCarDriftSpeedDownScale);
				}
				else
				{
					// 指数看的変化にする。
					float t = ((angleRate - kCarDriftSpeedDownRate) / kCarDriftSpeedDownRate);
					t = pow(t, 2.0f);
					m_moveSpeed = Math::Lerp<float>(min(t, 1.0f), kCarSpeed * kCarDriftSpeedDownScale, kCarSpeed);
				}

				// カーブの回転
				Quaternion curveRot = Quaternion::Identity;

				// 中心からのベクトルを直角に曲げて、それにドリフト分の角度を加味した回転を計算する。
				if (m_path.GetCurrentSectionTag() == nsAI::CPath::enLeftCurve)
				{
					// 左折時
					curveRot.SetRotationDegY(-90.0f - kCarDriftMaxAngle + addAngle);
				}
				else if (m_path.GetCurrentSectionTag() == nsAI::CPath::enRightCurve)
				{
					// 右折時
					curveRot.SetRotationDegY(90.0f + kCarDriftMaxAngle - addAngle);

				}
				curveRot.Apply(rotateVec);
			}
			else
			{
				// 通常は移動方向に回転させる
				rotateVec = m_moveDir;
			}

			// 次の回転
			Quaternion nextQRot;
			// 次の回転を計算
			nextQRot.SetRotation(Vector3::Front, rotateVec);
			// カクツキを抑えるために、球面線形補完を行う。
			nextQRot.Slerp(kCarModelRotRate, m_modelRender->GetRotation(), nextQRot);
			// モデルに回転を設定
			m_modelRender->SetRotation(nextQRot);

			return;
		}

		/**
		 * @brief サウンドを更新
		*/
		void CAICar::UpdateSound()
		{
			const float volume = CalcSoundVolume();
			m_driveSC->SetVolume(volume * kDriveSoundVolume);
			m_driftSC->SetVolume(volume * kDriftSoundVolume);

			if (m_driveTimer >= kDriveSoundTime)
			{
				m_driveSC->Play(false);
				m_driveTimer = 0.0f;
			}

			m_driveTimer += nsTimer::GameTime().GetFrameDeltaTime();

			if (m_path.GetCurrentSectionTag() != nsAI::CPath::enNone &&
				m_path.GetCurrentSectionTag() != nsAI::CPath::enStraight)
			{
				if (m_driftSoundFlag)
				{
					m_driftSC->Play(false);
					m_driftSoundFlag = false;
				}
			}
			else
			{
				m_driftSoundFlag = true;
			}

			return;
		}

		/**
		 * @brief サウンドの音量を計算する
		 * @return 計算されたサウンドの音量
		*/
		float CAICar::CalcSoundVolume()
		{
			Vector3 fromCamraVec = m_modelRender->GetPosition() - g_camera3D->GetPosition();

			float volume = max(0.0f,(kMaxSoundDistance - fromCamraVec.Length())) / kMaxSoundDistance;
			volume = pow(volume, 2.0f);

			return volume;
		}

		/**
		 * @brief このクラスのサウンドの停止
		*/
		void CAICar::StopSound()
		{
			// 再生中のサウンドをすべて停止

			if (m_driveSC->IsPlaying())
			{
				m_driveSC->Stop();
			}
			if (m_driftSC->IsPlaying())
			{
				m_driftSC->Stop();
			}

			return;
		}

		/**
		 * @brief 距離によるターゲットポイントの除外
		 * @param distance 自身とターゲットポイントの距離
		 * @return 除外したか？true戻ってきたら除外した
		*/
		bool CAICar::ExcludeForDistance(const float distance)
		{
			// 距離が一定以上のターゲットを除外
			if (distance > kBetweenTargetPoint)
			{
				return true;
			}

			// 距離が一定以下
			if (distance < kMinBetweenTargetPoint)
			{
				return true;
			}

			return false;
		}

		/**
		 * @brief 角度によるターゲットポイントの除外
		 * @param[out] moveToPTTAngleRad  移動方向とターゲット座標への方向のラジアン角
		 * @param[in] posToTargetPosDir 自身からターゲットへのベクトル
		 * @return 除外したか？trueが戻ってきたら除外した
		*/
		bool CAICar::ExcludeForAngle(float* moveToPTTAngleRadOut, const Vector3& posToTargetPosDir)
		{
			// 移動方向とターゲット座標への方向を比べる
			const float dotMoveAndPTTDir = Dot(m_moveDir, posToTargetPosDir);
			// 移動方向とターゲット座標への方向のラジアン角
			*moveToPTTAngleRadOut = acosf(dotMoveAndPTTDir);

			// 一定の角度より上のターゲットは対象外
			if (*moveToPTTAngleRadOut > kAngleThreshold)
			{
				return true;
			}

			return false;
		}

		/**
		 * @brief 候補に入れるターゲットポイントを調べる処理
		 * @param[in,out] candidateTPsOut ターゲットポイントの候補
		 * @param[in] targetPos ターゲットポイント
		 * @param[in,out] posToCandidateTPLenOut 自身から候補のターゲットポイントへの距離
		 * @param[in] posToTargetPosLen 自身からターゲットポイントへの距離
		 * @param[in] posToTargetPosDir 自身からターゲットポイントへの方向
		*/
		void CAICar::BeCandidateTargetPoint(
			std::unordered_map<nsAICharacterConstData::EnCandidateTargetPointType, Vector3>* candidateTPsOut,
			const Vector3& targetPos,
			float posToCandidateTPLenOut[nsAICharacterConstData::enCandidateTargetPointTypeNum],
			const float posToTargetPosLen,
			const Vector3& posToTargetPosDir
		)
		{
			for (int candidateTPTypeNum = 0; candidateTPTypeNum < enCandidateTargetPointTypeNum; candidateTPTypeNum++)
			{
				// 目標となる移動先への方向
				Vector3 moveTargetDir = Vector3::Front;
				float angleThreshold = kAngleThreshold;
				EnCandidateTargetPointType candidateTPType = static_cast<EnCandidateTargetPointType>(candidateTPTypeNum);

				switch (candidateTPType)
				{
				case enTurnRight:
					// moveTargetDir = Cross(m_moveDir, Vector3::Up);
					moveTargetDir = Cross(Vector3::Up, m_moveDir);
					break;
				case enTurnLeft:
					// moveTargetDir = Cross(Vector3::Up, m_moveDir);
					moveTargetDir = Cross(m_moveDir, Vector3::Up);
					break;
				case enGoStraight:
					moveTargetDir = m_moveDir;
					angleThreshold = kStraightAngleThreshold;
					break;
				}

				moveTargetDir.Normalize();

				// 目標となる移動先への方向とターゲット座標への方向を比べる
				const float dotMoveTargetAndPTTDir = Dot(moveTargetDir, posToTargetPosDir);
				// 目標となる移動先への方向とターゲット座標への方向のラジアン角
				const float moveTargetToPTTAngleRad = acosf(dotMoveTargetAndPTTDir);

				// 目標となる移動先への方向から一定以上角度があるターゲットポイントは対象外
				if (moveTargetToPTTAngleRad > angleThreshold)
				{
					continue;
				}

				// 始めての候補の場合
				if (candidateTPsOut->count(candidateTPType) == 0)
				{
					// 候補にする
					candidateTPsOut->insert({ candidateTPType, targetPos });
					// 距離を保持しておく
					posToCandidateTPLenOut[candidateTPType] = posToTargetPosLen;

					// 決まったから処理を終了する
					return;
				}

				// 既に候補がある場合

				// 候補のターゲットポイントを更新するか？
				bool doUpdateCandidateTP = false;
				switch (candidateTPType)
				{
				case enTurnRight:
					if (posToTargetPosLen > posToCandidateTPLenOut[candidateTPType])
					{
						// 右折は遠い方を候補にする
						doUpdateCandidateTP = true;
					}
					break;
				case enTurnLeft:
				case enGoStraight:
					if (posToTargetPosLen <= posToCandidateTPLenOut[candidateTPType])
					{
						// 左折と直進は近い方を候補にする
						doUpdateCandidateTP = true;
					}
					break;
				}

				if (doUpdateCandidateTP)
				{
					(*candidateTPsOut)[candidateTPType] = targetPos;
					// 距離も更新する
					posToCandidateTPLenOut[candidateTPType] = posToTargetPosLen;

					// 決まったから処理を終了する
					return;
				}

			}

			return;
		}

		/**
		 * @brief パスを構築する
		 * @param candidateTPType ターゲットポイントへの候補の種類
		 * @param targetPoint ターゲットポイント
		*/
		void CAICar::PathBuilding(
			const nsAICharacterConstData::EnCandidateTargetPointType candidateTPType,
			const Vector3& targetPoint
		)
		{
			// パスをクリア
			m_path.Clear();

			// スタート地点を追加
			m_path.AddPoint(m_modelRender->GetPosition());

			if (candidateTPType == enTurnLeft || candidateTPType == enTurnRight)
			{
				// 右折か左折なら中継地点を追加する

				// カーブの座標を作成する
				MakeCurve(candidateTPType, targetPoint);
			}

			// ゴール地点を追加
			m_path.AddPoint(targetPoint);

			// パスを構築
			m_path.Build();

			return;
		}

		/**
		 * @brief カーブの座標を作成する
		 * @param candidateTPType ターゲットポイントへの候補の種類
		 * @param targetPoint ターゲットポイント
		*/
		void CAICar::MakeCurve(
			const nsAICharacterConstData::EnCandidateTargetPointType candidateTPType,
			const Vector3& targetPoint
		)
		{
			// ターゲットポイントへのベクトル
			const Vector3 toTargetPointVec = targetPoint - m_modelRender->GetPosition();
			// ターゲットポイントへのベクトルが射影された移動方向ベクトル
			const float toTPVecProjMoveDir = Dot(m_moveDir, toTargetPointVec);

			enum EnControlPointType
			{
				enStartCP,
				enCenterCP,
				enMidleCP,
				enEndCP,
				enCPNum
			};

			// ベジェ曲線の制御点の座標
			Vector3 controlPoints[enCPNum];
			// 真ん中の制御点
			controlPoints[enCenterCP] = m_modelRender->GetPosition() + m_moveDir * toTPVecProjMoveDir;

			// 真ん中から最初の制御点へのベクトル
			Vector3 cCpToScpVec = m_modelRender->GetPosition() - controlPoints[enCenterCP];
			cCpToScpVec.Normalize();
			// 真ん中から最後の制御点へのベクトル
			Vector3 cCpToEcpVec = targetPoint - controlPoints[enCenterCP];
			cCpToEcpVec.Normalize();

			// 制御点間の長さ
			float cpLen = 875.0f;
			// 左折なら制御点間の長さが短い

			if (candidateTPType == enTurnRight)
			{
				// 右折なら制御点間の長さが長い
				cpLen = 1050.0f;
			}
			
			// 制御点へのベクトルを伸ばす
			cCpToScpVec.Scale(cpLen);
			cCpToEcpVec.Scale(cpLen * 1.5f);

			// 手前と奥の制御点の座標を計算する
			controlPoints[enStartCP] = controlPoints[enCenterCP] + cCpToScpVec;
			controlPoints[enEndCP] = controlPoints[enCenterCP] + cCpToEcpVec;
			controlPoints[enMidleCP] =
				controlPoints[enEndCP] + (cCpToScpVec * 0.1f) + (cCpToEcpVec * -0.5f);

			// カーブ時の中心座標を計算
			m_curveCenterPosition = controlPoints[enStartCP] + cCpToEcpVec;
			// カーブ時の中心座標からカーブ開始地点への座標を計算
			m_fromCurveCenterToCurveStartVec = cCpToEcpVec;
			m_fromCurveCenterToCurveStartVec.Scale(-1.0f);
			m_fromCurveCenterToCurveStartVec.Normalize();

			// カーブの開始地点と終了地点を計算
			Vector3 curveStartPos = controlPoints[enCenterCP] + (cCpToScpVec * 3.0f);
			Vector3 curveEndPos = controlPoints[enCenterCP] + (cCpToEcpVec * 1.5f);

			// 真ん中の制御点を移動させる
			controlPoints[enCenterCP] += (cCpToScpVec * -1.0f) + (cCpToEcpVec * 0.1f);
			//controlPoints[enStartCP] += (cCpToEcpVec * -0.2f);

			// ベジェ曲線クラス
			nsCurve::CBezierCurve bezierCurve;
			// ベジェ曲線に制御点を追加する
			for (int i = 0; i < enCPNum; i++)
			{
				bezierCurve.AddControlPoint(controlPoints[i]);
			}
			
			// パスのセクションのタグ
			nsAI::CPath::EnSectionTag startST = nsAI::CPath::enLeftCurveStart;
			nsAI::CPath::EnSectionTag sectionTag = nsAI::CPath::enLeftCurve;
			nsAI::CPath::EnSectionTag endST = nsAI::CPath::enLeftCurveEnd;

			if (candidateTPType == enTurnRight)
			{
				// 右折なら右折用のタグ
				startST = nsAI::CPath::enRightCurveStart;
				sectionTag = nsAI::CPath::enRightCurve;
				endST = nsAI::CPath::enRightCurveEnd;
			}

			// 中継地点をパスに追加
			
			// カーブ開始地点を追加
			m_path.AddPoint(curveStartPos, startST);
			// カーブ中の地点を追加
			for (int i = 0; i < kCurveRelayPointNum; i++)
			{
				if (i == (kCurveRelayPointNum - 1))
				{
					// 最後のカーブ地点には、カーブ終了のタグを埋め込む
					sectionTag = endST;
				}
				// 中継地点
				Vector3 relayPos = Vector3::Zero;
				// ベジェ曲線のパラメータ
				const float t = static_cast<float>(i) / static_cast<float>(kCurveRelayPointNum - 1);
				// ベジェ曲線をパラメータを指定して計算
				bezierCurve.CalcBezierCurve(t, &relayPos);
				// 中継地点をパスに追加
				m_path.AddPoint(relayPos, sectionTag);
			}
			// カーブ終了のセクションの終点を追加
			m_path.AddPoint(curveEndPos, nsAI::CPath::enStraight);

		}

	}
}