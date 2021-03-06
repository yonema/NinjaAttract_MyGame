#pragma once
/*!
 *@brief	ゲーム時間クラス。
 * シングルトン。
 */

#include "StopWatch.h"
#include <queue>
#include "Noncopyable.h"

namespace nsNinjaAttract
{
	/**
	 * @brief タイマー
	*/
	namespace nsTimer
	{

		/*!
		 *@brief	ゲーム時間。
		 */
		class CGameTime : private nsUtil::Noncopyable
		{
		public:
			CGameTime()
			{
				m_font.SetShadowParam(true, 2.0f, Vector4::Black);
			}
			~CGameTime() = default;
		public:  //メンバ関数

			/// <summary>
			/// ゲームタイムのインスタンスを取得
			/// </summary>
			/// <returns>ゲームタイムのインスタンス</returns>
			static CGameTime& GetInstance()
			{
				static CGameTime t;
				return t;
			}
			/*!
			 *@brief	1フレームの経過時間を取得(単位・秒)。
			 */
			const float GetFrameDeltaTime() const
			{
				//固定FPSにする。可変は要検討。
				//return 1.0f / 30.0f;
				return m_frameDeltaTime * m_updateSpeed;
			}

			void PushFrameDeltaTime(float deltaTime)
			{
				float a = max(1.0f / 144.0f, deltaTime);
				float b = min(a, 1.0f / 30.0f);
				m_frameDeltaTimeQue.push_back(min(max(1.0f / 144.0f, deltaTime), 1.0f / 30.0f));
				if (m_frameDeltaTimeQue.size() > 30.0f) {
					float totalTime = 0.0f;
					for (auto time : m_frameDeltaTimeQue) {
						totalTime += time;
					}
					//平均値をとる。
					m_frameDeltaTime = min(1.0f / 30.0f, totalTime / m_frameDeltaTimeQue.size());
					m_frameDeltaTimeQue.pop_front();
				}
			}

			/// <summary>
			/// FPSを描画する
			/// </summary>
			/// <param name="rc">レンダリングコンテキスト</param>
			/// <param name="elapsedTime">経過時間</param>
			void DrawFPS(RenderContext& rc, float elapsedTime);

			/**
			 * @brief 更新速度を設定する
			 * @param[in] updateSpeed 更新速度
			*/
			void SetUpdateSpeed(const float updateSpeed)
			{
				m_updateSpeed = updateSpeed;
			}

			/**
			 * @brief 更新速度を得る
			 * @return 更新速度
			*/
			float GetUpdateSPeed() const
			{
				return m_updateSpeed;
			}

		private:
			std::list<float> m_frameDeltaTimeQue;
			float m_frameDeltaTime = 1.0f / 60.0f;		//1フレームの経過時間。
			float m_timeTotal = 0.0f;	//カウントフレーム内の経過時間の総和
			float m_fps = 0.0f;			//表示するFPS
			Font m_font;				//フォント

			float m_updateSpeed = 1.0f;	// 更新速度
		};

		/// <summary>
		/// ゲームタイムのインスタンスを取得
		/// </summary>
		/// <returns>ゲームタイムのインスタンス</returns>
		static CGameTime& GameTime()
		{
			return CGameTime::GetInstance();
		}

	}
}