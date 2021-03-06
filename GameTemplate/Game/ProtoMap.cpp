#include "stdafx.h"
#include "ProtoMap.h"
#include "SkyCube.h"
#include "Player.h"
#include "Buildings.h"
#include "BuildingConstData.h"
#include "MapConstDatah.h"
#include "GameMainState.h"
#include "Goal.h"


namespace nsNinjaAttract
{
	/**
	 * @brief マップ（レベル）用ネームスペース
	*/
	namespace nsMaps
	{
		/**
		 * @brief テストマップ（レベル）用のネームスペース
		*/
		namespace nsProtoMaps
		{
			// マップの定数データを使用可能にする
			using namespace nsMapConstData;

			/**
			 * @brief スタート関数
			 * @return アップデートを行うか？
			*/
			bool CProtoMap::Start()
			{
				// ゲームステートの生成
				m_gameState = NewGO<nsGameState::CGameMainState>(nsCommonData::enPriorityFirst);

				// スカイキューブの生成と初期化
				m_skyCube = NewGO<nsNature::CSkyCube>(nsCommonData::enPriorityFirst);
				m_skyCube->Init(nsNature::nsSkyCubeConstData::enSkyCubeType_day);

				// プレイヤーの生成
				m_player = NewGO<nsPlayer::CPlayer>(nsCommonData::enPriorityFirst);

				// レベルの生成
				m_level3D.Init(
					kLevelFilePath[enLevelProto3],
					[&](nsLevel3D::SLevelObjectData& objData)
					{
						// 建物の生成
						if (objData.ForwardMatchName(nsBuilding::nsBuildingConstData::kBuildingForwardName))
						{
							// 建物のタイプの数の分、当たるまで全部調べる
							for (int i = 0; i < nsBuilding::nsBuildingConstData::enBuildingTypeNum; i++)
							{
								if (objData.EqualObjectName(
									nsBuilding::nsBuildingConstData::kBuildingNames[i]) != true
									)
								{
									// 名前が一致しなければ次へ
									continue;
								}

								// 名前が一致したら建物を生成する
								nsBuilding::CBuildings* building =NewGO<nsBuilding::CBuildings>(
									nsCommonData::enPriorityFirst,
									nsBuilding::nsBuildingConstData::kBuildingNames[i]
									);
								// 建物のタイプを指定して初期化
								//building->Init(
								//	static_cast<nsBuilding::nsBuildingConstData::EnBuildingType>(i),
								//	objData.position,
								//	objData.rotation
								//);
								return true;
							}

						}
						// プレイヤーの生成
						else if (objData.EqualObjectName(kPlayerName))
						{
							m_player->SetPosition(objData.position);
							m_player->SetRotation(objData.rotation);

							return true;
						}
						// ゴールの生成
						else if (objData.EqualObjectName(kGoalName))
						{
							m_goal = NewGO <nsGoal::CGoal>(nsCommonData::enPriorityFirst);
							m_goal->Init(objData.position, objData.rotation, objData.scale, *m_player);

							return true;
						}
						else if (objData.EqualObjectName("StreetTree"))
						{
							nsGraphic::nsModel::CModelRender* treeModel = 
								NewGO<nsGraphic::nsModel::CModelRender>(nsCommonData::enPriorityFinal,"StreetTree");
							treeModel->SetPosition(objData.position);
							treeModel->SetRotation(objData.rotation);
							treeModel->SetScale(objData.scale);
							treeModel->IniTranslucent("Assets/modelData/streetElements/StreetTree.tkm");
							return true;
						}

						return false;
					}
				);


				// タイマーの計測を始める
				m_gameState->StartTimingGame();

				return true;
			}

			/**
			 * @brief 破棄した時に呼ばれる関数
			*/
			void CProtoMap::OnDestroy()
			{
				DeleteGO(m_skyCube);	// スカイキューブクラスの破棄
				DeleteGO(m_player);		// プレイヤークラスの破棄
				DeleteGO(m_goal);		// ゴールを破棄

				// 全てのタイプの建物を破棄
				for (int i = 0; i < nsBuilding::nsBuildingConstData::enBuildingTypeNum; i++)
				{
					// 建物をすべて破棄
					QueryGOs<nsBuilding::CBuildings>(
						nsBuilding::nsBuildingConstData::kBuildingNames[i],
						[&](nsBuilding::CBuildings* building)->bool
						{
							DeleteGO(building);
							return true;
						}
					);
				}

				QueryGOs<nsGraphic::nsModel::CModelRender>(
					"StreetTree",
					[](nsGraphic::nsModel::CModelRender* treeModel)->bool
					{
						DeleteGO(treeModel);
						return true;
					}
				);

				DeleteGO(m_gameState);

				return;
			}

			/**
			 * @brief アップデート関数
			*/
			void CProtoMap::Update()
			{
				return;
			}
		}
	}
}