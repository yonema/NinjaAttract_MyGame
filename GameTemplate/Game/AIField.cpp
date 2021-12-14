#include "stdafx.h"
#include "AIField.h"
#include "AIConstData.h"
#include "ModelRenderConstData.h"
#include "Level3D.h"

namespace nsMyGame
{
	/**
	 * @brief AI関連のネームスペース
	*/
	namespace nsAI
	{
		// AIの定数データを使用可能にする
		using namespace nsAIConstData;

		/**
		 * @brief コンストラクタ
		*/
		CAIField::CAIField()
		{
			// 初期化
			Init();

			return;
		}


		/**
		 * @brief 初期化
		*/
		void CAIField::Init()
		{
			// ナビメッシュの生成
			m_naviMesh.Init(kNaviMeshFilePath);
			
			// ナビメッシュのターゲットポイント用のレベル
			nsLevel3D::CLevel3D naviMeshTargetPointLevel;
			naviMeshTargetPointLevel.Init(
				kNaviMeshTargetPointLevelFilePath,
				[&](nsLevel3D::SLevelObjectData& objData)
				{
					// ターゲットポイントを格納していく
					m_naviMeshTargetPoints.emplace_back(objData.position);
					return true;
				}
			);


			m_AICharaInitData.naviMeshRef = &m_naviMesh;
			m_AICharaInitData.pathFindingRef = &m_pathFinding;
			m_AICharaInitData.naviMeshTargetPointsRef = &m_naviMeshTargetPoints;

			return;
		}

	}
}