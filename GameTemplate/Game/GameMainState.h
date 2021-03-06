#pragma once
#include "GameMainStateConstData.h"
#include "StopWatch.h"
#include "UIConstData.h"

namespace nsNinjaAttract
{
	// 前方宣言
	namespace nsUI { class CGameMainUI; }	//!< ゲームメインのUIクラス
	namespace nsPlayer { class CPlayer; }	//!< プレイヤークラス
	namespace nsAICharacter { class CAICar; } //!< 車クラス

	/**
	 * @brief ゲームステート関連のネームスペース
	*/
	namespace nsGameState
	{
		/**
		 * @brief ゲームメインのステートクラス
		*/
		class CGameMainState : public IGameObject
		{
		public:		// コンストラクタとデストラクタ
			/**
			 * @brief コンストラクタ
			*/
			CGameMainState()
			{
				m_instance = this;
			}
			/**
			 * @brief デストラクタ
			*/
			~CGameMainState() = default;

		public:		// オーバーライドしたメンバ関数

			/**
			 * @brief Updateの直前で呼ばれる開始処理
			 * @return アップデートを行うか？
			*/
			bool Start() override final;

			/**
			 * @brief 更新処理
			*/
			void OnDestroy() override final;

			/**
			 * @brief 消去される時に呼ばれる処理
			*/
			void Update() override final;
		public:		// メンバ関数

			/**
			 * @brief 初期化処理
			 * @param[in] player プレイヤーの参照
			*/
			void Init(const nsPlayer::CPlayer& player)
			{
				m_playerRef = &player;
			}

			/**
			 * @brief ゲームのタイムの計測開始
			*/
			void StartTimingGame()
			{
				m_isTimeGame = true;	// タイムを計る
				m_stopWatch.Start();	// ストップウォッチの計測開始
			}

			/**
			 * @brief ゲームのタイムの計測終了
			*/
			void StopTimingGame()
			{
				m_isTimeGame = false;	// タイムを計らない
				m_stopWatch.Stop();		// ストップウォッチの計測終了
			}

			/**
			 * @brief ゲームのタイムを得る
			 * @return 
			*/
			float GetGameTime() const
			{
				return m_gameTimer;
			}

			/**
			 * @brief プレイヤーの参照を得る
			 * @return プレイヤーの参照
			*/
			const nsPlayer::CPlayer& GetPlayer() const
			{
				return *m_playerRef;
			}

			/**
			 * @brief 車を追加する
			 * @param[in,out] aiCar 追加する車
			*/
			void AddAICar(nsAICharacter::CAICar* aiCar);

			/**
			 * @brief 車達の参照を得る
			 * @return 車たちの参照
			*/
			std::vector<nsAICharacter::CAICar*>* GetAICar()
			{
				return &m_aiCarsRef;
			}

			/**
			 * @brief ミッションを一つクリアする
			 * @param[in] missionType クリアするミッションのタイプ
			*/
			void ClearOneMission(const nsUI::nsMissionUIConstData::EnMissionType missionType);

			/**
			 * @brief クリアフラグを得る。要素の数はnsMissionUIConstData::enMissionTypeNum。
			 * @return クリアフラグ
			*/
			const bool* GetClearFlag() const
			{
				return m_missionClearFlag;
			}

			/**
			 * @brief ステートを遷移
			 * @param[in] newState 新しいステート
			*/
			void ChangeState(const nsGameMainStateConstData::EnGameMainStateState newState);

			/**
			 * @brief ゲームメインステートのステートを得る
			 * @return ゲームメインステートのステート
			*/
			nsGameMainStateConstData::EnGameMainStateState GetGameMainStateState() const
			{
				return m_gameMainStateState;
			}

			/**
			 * @brief コマンド入力のミスをカウント
			*/
			void CountMissCommand()
			{
				m_numOfCommandMiss++;
			}

			/**
			 * @brief コマンドミスの回数を得る
			 * @return コマンドミスの回数
			*/
			int GetNumOfCommandMiss() const
			{
				return m_numOfCommandMiss;
			}

			/**
			 * @brief ミッションを表示する
			*/
			void ShowMission();

		public:		// staticなメンバ関数とデータメンバ

			static CGameMainState* m_instance;	//!< インスタンス

			/**
			 * @brief インスタンスを得る
			 * @return インスタンス
			*/
			static CGameMainState* GetInstance()
			{
				return m_instance;
			}

		private:	// privateなメンバ関数

			/**
			 * @brief ゲームのタイムを計測する
			*/
			void TimeGame();

		private:	// データメンバ

			//!< ゲームメインステートのステート
			nsGameMainStateConstData::EnGameMainStateState m_gameMainStateState =
				nsGameMainStateConstData::enGS_startDirecting;
			float m_gameTimer = 0.0f;	//!< ゲームのタイマー
			bool m_isTimeGame = false;	//!< ゲームのタイムを計るか？
			nsTimer::CStopWatch m_stopWatch;	//!< ストップウォッチ

			nsUI::CGameMainUI* m_gameMainUI = nullptr;	//!< ゲームメインのUIクラス

			const nsPlayer::CPlayer* m_playerRef = nullptr;	//!< プレイヤーのconst参照
			std::vector<nsAICharacter::CAICar*> m_aiCarsRef;	//!< 車達の参照

			//!< ミッションのクリアフラグ
			bool m_missionClearFlag[nsUI::nsMissionUIConstData::enMissionTypeNum] = {};
			int m_clearCounter = 0;		//!< クリアカウンター
			float m_directingTimer = 0.0f;	//!< 演出用のタイマー
			int m_numOfCommandMiss = 0;	//!< コマンド入力をミスした回数
			
		};

		/**
		 * @brief ゲームメインステートのインスタンスを得る
		 * @return ゲームメインステートのインスタンス
		*/
		static CGameMainState* GameMainState()
		{
			return CGameMainState::GetInstance();
		}

	}
}