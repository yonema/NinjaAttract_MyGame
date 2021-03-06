#pragma once

namespace nsNinjaAttract
{
	// 前方宣言
	namespace nsPlayer { class CPlayer; }			// プレイヤークラス
	namespace nsGameState { class CGameMainState; }	// ゲームメインステートクラス
	namespace nsBGM { class CBGM; }					// BGMクラス
	namespace nsBackGround { class CBackGround; }	// バックグラウンドクラス
	namespace nsAI { class CAIField; }				// AIフィールドクラス
	namespace nsLevel3D { class CLevel3D; }			// 3Dレベルクラス
	namespace nsSound { class CSoundCue; }			// サウンド

	/**
	 * @brief マップ（レベル）用ネームスペース
	*/
	namespace nsMaps
	{
		/**
		 * @brief メインマップクラス
		*/
		class CMainMap : public IGameObject
		{
		public:		// コンストラクタとデストラクタ
			/**
			 * @brief コンストラクタ
			*/
			CMainMap() = default;
			/**
			 * @brief デストラクタ
			*/
			~CMainMap() = default;

		public:		// オーバーライドしたメンバ関数

			/**
			 * @brief Updateの直前で呼ばれる開始処理
			 * @return アップデートを行うか？
			*/
			bool Start() override final;

			/**
			 * @brief 消去される時に呼ばれる処理
			*/
			void OnDestroy() override final;

			/**
			 * @brief 更新処理
			*/
			void Update() override final;

		public:		// メンバ関数

			/**
			 * @brief 背景ステージクラスを設定する
			 * @param[in,out] backGround 背景ステージクラス
			*/
			void SetBackGround(nsBackGround::CBackGround* backGround)
			{
				m_backGround = backGround;
			}

		private:	// privateなメンバ関数

			/**
			 * @brief プレイヤーの初期化
			*/
			void InitPlayer();

			/**
			 * @brief 車の初期化
			*/
			void InitCar();

			/**
			 * @brief 開始演出の更新
			*/
			void UpdateStartDirecting();

			/**
			 * @brief ゲーム中の更新
			*/
			void UpdateInGame();

			/**
			 * @brief クリア演出の前のフェードアウトの更新
			*/
			void UpdateFadeOutToClearDirecting();

			/**
			 * @brief 最後のジャンプの更新
			*/
			void UpdateLastJump();

			/**
			 * @brief タイトルへ遷移の更新
			*/
			void UpdateGoTitle();

		private:	// データメンバ
			std::unique_ptr<nsLevel3D::CLevel3D> m_playerLevel;	//!< プレイヤー用レベル
			std::unique_ptr<nsLevel3D::CLevel3D> m_carLevel;	//!< 車用レベル
			nsPlayer::CPlayer* m_player = nullptr;				//!< プレイヤークラス
			nsGameState::CGameMainState* m_gameState = nullptr;	//!< ゲームステートクラス
			nsBGM::CBGM* m_bgm = nullptr;						//!< BGMクラス
			nsBackGround::CBackGround* m_backGround = nullptr;	//!< バックグラウンドクラス
			nsAI::CAIField* m_aiField = nullptr;				//!< AIのフィールドクラス
			float m_directingTimer = 0.0f;						//!< 演出用タイマー
			nsSound::CSoundCue* m_decisionSC = nullptr;			//!< 決定音のサウンド
			nsSound::CSoundCue* m_startVoiceSC = nullptr;		//!< 開始ボイスのサウンド
		};

	}
}