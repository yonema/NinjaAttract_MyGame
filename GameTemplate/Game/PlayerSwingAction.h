#pragma once
#include "Noncopyable.h"
#include "PlayerConstData.h"


namespace nsNinjaAttract
{		
	// 前方宣言
	namespace nsPlayer
	{
		class CPlayer;			// プレイヤークラス
		class CPlayerCamera;	// プレイヤーのカメラクラス
		class CPlayerModelAnimation; // プレイヤーモデルアニメーションクラス
	}
	namespace nsSound { class CSoundCue; }


	/**
	 * @brief プレイヤー関連のネームスペース
	*/
	namespace nsPlayer
	{
		/**
		 * @brief プレイヤーの動き関連のネームスペース
		*/
		namespace nsPlayerMovenent
		{
			// 前方宣言
			class CPlayerMovement;	// プレイヤーの動きクラス

			/**
			 * @brief プレイヤーのスイングのアクションクラス
			*/
			class CPlayerSwingAction : private nsUtil::Noncopyable
			{
			private:	// エイリアス宣言
				// スイングアクションのステート
				using EnSwingActionState = nsPlayerConstData::nsSwingActionConstData::EnSwingActionState;

			public:		// コンストラクタとデストラクタ
				/**
				 * @brief コンストラクタ
				*/
				CPlayerSwingAction();
				/**
				 * @brief デストラクタ
				*/
				~CPlayerSwingAction();

			public:		// メンバ関数

				/**
				 * @brief 初期化
				 * @param[in.out] player プレイヤー
				 * @param[in,out] playerMovement プレイヤー移動クラスの参照
				 * @param[in,out] playerCamera プレイヤーカメラクラスの参照
				 * @param[in.out] playerModelAnimation プレイヤーモデルアニメーションの参照
				*/
				void Init(
					CPlayer* player,
					CPlayerMovement* playerMovement,
					CPlayerCamera* playerCamera,
					CPlayerModelAnimation* playerModelAnimation
				);

				/**
				 * @brief スイングアクションを実行
				*/
				void Execute();

				/**
				 * @brief スイングアクションの前に行う処理
				*/
				void PreSwingAction();

				/**
				 * @brief スイング中の速度をリセットする
				*/
				void ResetSwingSpeed()
				{
					m_swingSpeed = 0.0f;
					m_swingSpeed = -100.0f;
				}

				/**
				 * @brief スイング中のスピードを得る
				 * @return スイング中のスピード
				*/
				float GetSwingSpeed() const
				{
					return m_swingSpeed;
				}

				/**
				 * @brief 初期状態に戻す
				*/
				void Reset()
				{
					EndSwing();
				}

			private:	// privateなメンバ関数

				/**
				 * @brief サウンドを初期化
				*/
				void InitSound();

				/**
				 * @brief スイングターゲットを探す
				*/
				void FindSwingTarget();

				/**
				 * @brief 糸を伸ばしている最中の処理
				*/
				void StringStretching();

				/**
				 * @brief スイングアクションの処理
				*/
				void SwingAction();

				/**
				 * @brief スイング後の空中の処理
				*/
				void AirAfterSwing();

				/**
				 * @brief スイング処理の終了
				*/
				void EndSwing();

				/**
				 * @brief スイングアクションステートを変更する
				 * @param[in] swingActionState スイングアクションのステート
				*/
				void ChangeState(const EnSwingActionState swingActionState);

				/**
				 * @brief 糸を伸ばし中に遷移するときに一度だけ呼ばれるイベント
				*/
				void IsStringStretchingEvent();

				/**
				 * @brief スイング中に遷移するときに一度だけ呼ばれるイベント
				*/
				void IsSwingingEvent();

				/**
				 * @brief スイング後の空中に遷移するときに一度だけ呼ばれるイベント
				*/
				void IsAirAfterSwingEvent();

				/**
				 * @brief カメラの値を線形変化させる
				*/
				void CameraChangeLinearly();

			private:	// データメンバ
				CPlayer* m_playerRef = nullptr;					//!< プレイヤークラスの参照
				CPlayerMovement* m_playerMovementRef = nullptr;	//!< プレイヤー移動クラスの参照
				CPlayerCamera* m_playerCameraRef = nullptr;		//!< プレイヤーカメラクラスの参照
				CPlayerModelAnimation* m_playerModelAnimationRef = nullptr;	//!< プレイヤーのモデルアニメーションの参照
				const Vector3* m_swingTargetPos = nullptr;		//!< スイングターゲットの座標
				//!< スイングアクションのステート
				EnSwingActionState m_swingActionState = 
					nsPlayerConstData::nsSwingActionConstData::enFindSwingTarget;
				float m_swingRadAngle = 0.0f;						//!< スイングのラジアン角度
				float m_swingSpeed = 0.0f;							//!< スイングスピード
				//!< 減速し始めるスイングスピード
				float m_startDecelerateSwingSpeed = 
					nsPlayerConstData::nsSwingActionConstData::kStartDecelerateSwingSpeedInitialValue;
				Vector3 m_inputMoveDirXZ = Vector3::Zero;			//!< 入力によって生じたXZ平面での移動方向
				float m_velocityAfterSwing = 0.0f;					//!< スイング後の速度
				float m_accelerationAfterSwing = 0.0f;				//!< スイング後の加速
				float m_g = 0.0f;
				float m_cameraChangeLinearlyTimer = 0.0f;			//!< カメラの値の線形変化用のタイマー
				bool m_afterSwing = false;							//!< スイングを行ったか？
				bool m_swingRollFlag = false;						//!< スイングロールを行うか？
				Vector3 m_swingForwardDir = Vector3::Front;			//!< スイングの前方向

				// サウンド
				nsSound::CSoundCue* m_chainPutOutSC = nullptr;		//!< 鎖を出すのサウンド
				nsSound::CSoundCue* m_chainBendingSC = nullptr;		//!< 鎖がしなるのサウンド
				nsSound::CSoundCue* m_chainReleaseSC = nullptr;		//!< 鎖を離すのサウンド
				nsSound::CSoundCue* m_swingLeaveSC = nullptr;		//!< スイング状態から離れるのサウンド
				nsSound::CSoundCue* m_swingRollLeaveSC = nullptr;	//!< スイングロール状態から離れるのサウンド
				//!< スイングのボイスのサウンド
				nsSound::CSoundCue*
					m_swingVoiceSC[nsPlayerConstData::nsWalkAndRunConstData::kJumpVoiceTypeNum] = {};

				std::unique_ptr<std::mt19937> m_mt;							//!< メルセンヌツイスターの32ビット版
				std::unique_ptr<std::uniform_int_distribution<>> m_rand;	//!< 範囲付きの一様乱数

			};
		}
	}
}
