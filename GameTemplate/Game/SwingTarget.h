#pragma once
#include "Noncopyable.h"
#include "AABB.h"
#include "ModelRender.h"


namespace nsMyGame
{

	/**
	 * @brief 糸を使ったアクションのターゲット関連のネームスペース
	*/
	namespace nsStringActionTarget
	{

		/**
		 * @brief スイングのターゲット関連のネームスペース
		*/
		namespace nsSwingTarget
		{
			/**
			 * @brief スイングのターゲットクラス
			*/
			class CSwingTarget : private nsUtil::Noncopyable
			{
			public:		// コンストラクタとデストラクタ
				/**
				 * @brief コンストラクタ
				*/
				CSwingTarget() = default;
				/**
				 * @brief デストラクタ
				*/
				~CSwingTarget();

			public:		// メンバ関数

				/**
				 * @brief 初期化
				 * @param[in] modelRender モデルレンダラー
				*/
				void Init(const nsGraphic::nsModel::CModelRender& modelRender);


				/**
				 * @brief スイングのターゲットの座標を計算する
				 * @derails 引数が (1, 1, 1) でターゲットの数は8つ（8頂点分）
				 * @param[in] widthSegments 幅セグメント ( X )
				 * @param[in] heightSegments 高さセグメント ( Y )
				 * @param[in] lenghtSegments 長さセグメント ( Z )
				*/
				void CalcSwingingTargetPositions(
					const UINT widthSegments,
					const UINT heightSegments,
					const UINT lengthSegments
				);

				/**
				 * @brief スイングのターゲットの座標コンテナを取得
				 * @return スイングのターゲットの座標コンテナ
				*/
				const std::vector<Vector3>& GetSwingTargetPositions() const
				{
					return m_swingTargetPositions;
				}

				/**
				 * @brief 座標を得る
				 * @return 座標
				*/
				const Vector3& GetPosition() const
				{
					return m_modelRender->GetPosition();
				}

			private:	// privateなメンバ関数

				/**
				 * @brief スイングのターゲットの座標コンテナを再初期化する
				 * @param[in] widthSegments 幅セグメント ( X )
				 * @param[in] heightSegments 高さセグメント ( Y )
				 * @param[in] lenghtSegments 長さセグメント ( Z )
				*/
				void ReInitSwingTargetPositions(
					const UINT widthSegments,
					const UINT heightSegments,
					const UINT lengthSegments
				);

			private:	// データメンバ
				nsGeometry::CAABB m_aabb;							//!< AABBクラス
				std::vector<Vector3> m_swingTargetPositions;		//!< スイングのターゲットの座標コンテナ
				const nsGraphic::nsModel::CModelRender* m_modelRender = nullptr;	//!< モデルレンダラー
			};

		}
	}
}



