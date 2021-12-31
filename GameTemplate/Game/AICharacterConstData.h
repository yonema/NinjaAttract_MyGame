#pragma once

namespace nsMyGame
{

	/**
	 * @brief AIキャラクター関連のネームスペース
	*/
	namespace nsAICharacter
	{
		/**
		 * @brief AIキャラクターの定数データ
		*/
		namespace nsAICharacterConstData
		{
			// 車のモデルデータのファイルパス
			constexpr const char* const kCarModelFilePath = "Assets/modelData/car/sedan_car.tkm";

			//!< 爆発のエフェクトのファイルパス
			constexpr const char16_t* const kExplotionEffectFilePath = u"Assets/effect/explosion.efk";
			// 爆発のエフェクトが出る座標の、高さのバッファ
			constexpr float kExplotionPosBufHeight = 50.0f;
			//!< 爆発のエフェクトの拡大率
			static const Vector3 kExplotionScale = { 2.0f,1.0f,2.0f };

			//!< 煙のエフェクトのファイルパス
			constexpr const char16_t* const kSmokeEffectFilePath = u"Assets/effect/smoke.efk";
			constexpr int kSmokeNum = 20;	//!< 煙の数
			constexpr float kSmokeTime = 2.0f;	//!< 煙の時間
			// 煙のエフェクトが出る座標の、前向きのバッファ
			constexpr float kSmokePosBufForward = 100.0f;
			// 煙のエフェクトが出る座標の、高さのバッファ
			constexpr float kSmokePosBufHeight = 100.0f;

			//!< 爆発時の煙のエフェクトのファイルパス
			constexpr const char16_t* const kSmokeExplosionEffectFilePath = u"Assets/effect/smokeExplosion.efk";

			constexpr float kCarSpeed = 2500.0f;	//!< 車の速度
			constexpr float kCarDriftMaxAngle = 90.0f;	//!< 車のドリフトの最大角度
			constexpr float kCarModelRotRate = 0.2f;	//!< 車のモデルの球面線形補完率
			constexpr float kCarDriftTurnBackRate = 0.6f;	//!< 車のドリフトの回転を戻し始めるレート
			constexpr float kCarDriftSpeedDownScale = 0.7f;	//!< 車のドリフトのスピードダウン率
			constexpr float kCarDriftSpeedDownRate = 0.5f;	//!< 車のドリフトのスピードが落ち始めるレート

			// 実際のターゲットポイントの間隔
			constexpr float kRealBetweenTargetPoint = 11200.0f;
			// ターゲットポイントの感覚のゆとり
			constexpr float kBetweenTargetPointBuff = 1000.0f;
			// 使用するターゲットポイントの間隔
			constexpr float kBetweenTargetPoint = kRealBetweenTargetPoint + kBetweenTargetPointBuff;
			// 使用するターゲットポイントの最低間隔
			constexpr float kMinBetweenTargetPoint = 100.0f;
			// 角度のしきい値
			// 1.22rad≒70.0deg
			constexpr float kAngleThreshold = 1.22f;
			// 直進用の角度のしきい値
			// 0.52rad≒30.0deg
			constexpr float kStraightAngleThreshold = 0.52f;

			constexpr int kCurveRelayPointNum = 10;	//!< カーブの中継地点の数

			/**
			 * @brief ターゲットポイントの候補の種類
			*/
			enum EnCandidateTargetPointType
			{
				enTurnRight,					//!< 右折
				enTurnLeft,						//!< 左折
				enGoStraight,					//!< 直進
				enCandidateTargetPointTypeNum	//!< 候補の種類の数
			};
		}
	}
}