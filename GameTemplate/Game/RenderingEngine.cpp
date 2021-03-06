#include "stdafx.h"
#include "RenderingEngine.h"
#include "system/system.h"
#include "GameTime.h"
#include "StopWatch.h"
#include "Render.h"
#include "LightManager.h"
#include "SpriteRender.h"
#include "Fade.h"

namespace nsNinjaAttract
{

	/**
	 * @brief このゲーム用のゲームエンジンネームスペース
	*/
	namespace nsMyEngine
	{
		CRenderingEngine* CRenderingEngine::m_instance = nullptr;	// 唯一のインスタンス

		// レンダリングエンジンの定数データを使用可能にする
		using namespace nsRenderingEngineConstData;

		/**
		 * @brief コンストラクタ
		*/
		CRenderingEngine::CRenderingEngine()
		{
			// 描画できるオブジェクトのキャパシティを確保する
			m_renderObjects.reserve(m_kRenderObjectsCapacityNum);

			InitMainRenderTarget();	// メインレンダリングターゲットの初期化
			InitGBuffer();	// GBufferを初期化
			// メインレンダリングターゲットをフレームバッファにコピーするためのスプライトの初期化
			InitCopyMainRenderTargetToFrameBufferSprite();
			InitShadowMapRender();	// シャドウマップレンダラーの初期化
			InitDefferdLightingSprite();	// ディファ―ドライティングを行うためのスプライトの初期化

			m_postEffect.Init(m_mainRenderTarget);	// ポストエフェクトの初期化

			// フェードクラスの生成
			m_fade = NewGO<nsGraphic::CFade>(nsCommonData::enPriorityFirst);

			return;
		}
		/**
		 * @brief デストラクタ
		*/
		CRenderingEngine::~CRenderingEngine()
		{

			return;
		}

		/**
		 * @brief メインレンダリングターゲットの初期化
		*/
		void CRenderingEngine::InitMainRenderTarget()
		{
			m_mainRenderTarget.Create(
				g_graphicsEngine->GetFrameBufferWidth(),
				g_graphicsEngine->GetFrameBufferHeight(),
				1,
				1,
				DXGI_FORMAT_R16G16B16A16_FLOAT,	// 16bit浮動小数点型
				DXGI_FORMAT_UNKNOWN
			);
			return;
		}

		/**
		 * @brief GBufferを初期化
		*/
		void CRenderingEngine::InitGBuffer()
		{
			int frameBuffer_w = g_graphicsEngine->GetFrameBufferWidth();
			int frameBuffer_h = g_graphicsEngine->GetFrameBufferHeight();

			// アルベドカラーを出力用のレンダリングターゲットを初期化する
			m_GBuffer[enGBufferAlbedoDepth].Create(
				frameBuffer_w,
				frameBuffer_h,
				1,
				1,
				DXGI_FORMAT_R32G32B32A32_FLOAT,	// 32bit浮動小数点型
				DXGI_FORMAT_D32_FLOAT
			);

			// 法線出力用のレンダリングターゲットを初期化する
			m_GBuffer[enGBufferNormal].Create(
				frameBuffer_w,
				frameBuffer_h,
				1,
				1,
				DXGI_FORMAT_R8G8B8A8_SNORM,	// 8bit符号付き正規化整数
				DXGI_FORMAT_UNKNOWN
			);


			// メタリック、影パラメータ、スムース出力用のレンダリングターゲットを初期化する    
			m_GBuffer[enGBufferMetaricShadowSmooth].Create(
				frameBuffer_w,
				frameBuffer_h,
				1,
				1,
				DXGI_FORMAT_R8G8B8A8_UNORM,	// 8bit符号なし正規化整数
				DXGI_FORMAT_UNKNOWN
			);

			return;
		}

		/**
		 * @brief メインレンダリングターゲットをフレームバッファにコピーするためのスプライトの初期化
		*/
		void CRenderingEngine::InitCopyMainRenderTargetToFrameBufferSprite()
		{
			SpriteInitData spriteInitData;

			// テクスチャはメインレンダリングターゲット
			spriteInitData.m_textures[0] = &m_mainRenderTarget.GetRenderTargetTexture();

			// レンダリング先がフレームバッファーなので、解像度はフレームバッファーと同じ
			spriteInitData.m_width = g_graphicsEngine->GetFrameBufferWidth();
			spriteInitData.m_height = g_graphicsEngine->GetFrameBufferHeight();
			spriteInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

			// ガンマ補正ありの2D描画のシェーダーを指定する
			spriteInitData.m_fxFilePath = m_kSpriteFxFilePath;

			// 初期化オブジェクトを使って、スプライトを初期化する
			m_copyMainRtToFrameBufferSprite.Init(spriteInitData);

			return;
		}

		/**
		 * @brief シャドウマップレンダラーの初期化
		*/
		void CRenderingEngine::InitShadowMapRender()
		{
			// シャドウマップレンダラーの初期化
			for (auto& shadowMapRender : m_shadowMapRenders) 
			{
				shadowMapRender.Init();
			}

			return;
		}

		/**
		 * @brief ディファ―ドライティングを行うためのスプライトの初期化
		*/
		void CRenderingEngine::InitDefferdLightingSprite()
		{
			// スプライトを初期化データ
			SpriteInitData spriteInitData;

			// 画面全体にレンダリングするので幅と高さはフレームバッファーの幅と高さと同じ
			spriteInitData.m_width = g_graphicsEngine->GetFrameBufferWidth();
			spriteInitData.m_height = g_graphicsEngine->GetFrameBufferHeight();

			// fxファイルパスを設定
			spriteInitData.m_fxFilePath = m_kDefferdLightingSpriteFxFilePath;

			// 定数バッファの登録
			// ライト情報を登録
			spriteInitData.m_expandConstantBuffer[0] = 
				&nsLight::CLightManager::GetInstance()->GetLightData();
			spriteInitData.m_expandConstantBufferSize[0] =
				sizeof(nsLight::CLightManager::GetInstance()->GetLightData());
			// ディファードレンダリング用の定数バッファを登録
			spriteInitData.m_expandConstantBuffer[1] = &m_defferdLightingCB;
			spriteInitData.m_expandConstantBufferSize[1] = sizeof(m_defferdLightingCB);
			// IBL用の定数バッファを登録
			spriteInitData.m_expandConstantBuffer[2] = &m_IBLCB;
			spriteInitData.m_expandConstantBufferSize[2] = sizeof(m_IBLCB);

			// ディファードライティングで使用するテクスチャを設定

			// テクスチャの番号
			int texNo = 0;

			// GBufferのレンダリングターゲットのテクスチャを設定する
			for (auto& GBuffer : m_GBuffer)
			{
				spriteInitData.m_textures[texNo++] = &GBuffer.GetRenderTargetTexture();
			}

			// シャドウマップのテクスチャを設定する
			for (int i = 0; i < nsLight::nsLightConstData::kMaxDirectionalLightNum; i++)
			{
				for (int areaNo = 0; areaNo < nsGraphic::nsShadow::nsShadowConstData::enShadowMapArea_num; areaNo++)
				{
					spriteInitData.m_textures[texNo++] = &m_shadowMapRenders[i].GetShadowMap(areaNo);
				}
			}

			// プレイヤー専用のシャドウマップのテクスチャを設定する
			for (int i = 0; i < nsLight::nsLightConstData::kMaxDirectionalLightNum; i++)
			{
				spriteInitData.m_textures[texNo++] = &m_shadowMapRenders[i].GetPlayerShadowMap();
			}


			// IBLを行うか？、かつ
			// IBLに使用するテクスチャが有効か？
			if (m_IBLCB.isIBL == true && m_IBLTexture.IsValid())
			{
				// IBLに使用するテクスチャを設定する
				spriteInitData.m_textures[texNo++] = &m_IBLTexture;
			}


			// メインレンダリングターゲットに描画するため
			// メインレンダリングターゲットとカラーフォーマットを合わせる
			spriteInitData.m_colorBufferFormat[0] = m_mainRenderTarget.GetColorBufferFormat();

			// 初期化データを使ってスプライトを生成して初期化
			// ReInitする時用にユニークポインタを使ってリソースを開放して、新たなリソースの所有権を得る
			m_diferredLightingSprite.reset(new Sprite);
			m_diferredLightingSprite->Init(spriteInitData);

			return;
		}

		/**
		 * @brief IBLを再初期化する
		 * @param[in] ddsFilePath IBLのテクスチャのddsファイルパス
		 * @param[in] luminance IBLの明るさ
		 * @param[in] isIBL IBLを行うか？
		*/
		void CRenderingEngine::ReInitIBL(const wchar_t* ddsFilePath, const float luminance, const bool isIBL)
		{
			// IBLのデータを初期化する
			InitIBLData(ddsFilePath, luminance);

			// IBLを行うか？を設定
			m_IBLCB.isIBL = isIBL;

			// ディファードライティングを行うためのスプライトを初期化する
			InitDefferdLightingSprite();

			return;
		}

		/**
		 * @brief ビューカリング用のビュープロジェクション行列を計算
		*/
		void CRenderingEngine::CalcViewProjectionMatrixForViewCulling()
		{
			Matrix projMatrix;
			projMatrix.MakeProjectionMatrix(
				g_camera3D->GetViewAngle() * 1.0f,
				g_camera3D->GetAspect(),
				g_camera3D->GetNear(),
				g_camera3D->GetFar()
			);
			m_viewProjMatrixForViewCulling.Multiply(g_camera3D->GetViewMatrix(), projMatrix);
			//Camera cam;
			//cam.SetViewAngle(g_camera3D->GetViewAngle());
			//cam.SetNear(g_camera3D->GetNear());
			//cam.SetFar(g_camera3D->GetFar());
			//cam.SetPosition(Vector3::Zero);
			//cam.SetTarget(Vector3::Front * 100.0f);
			//cam.Update();
			//m_viewProjMatrixForViewCulling.Multiply(cam.GetViewMatrix(), projMatrix);

			return;
		}

		/**
		 * @brief 事前破棄処理
		*/
		void CRenderingEngine::PreDelete()
		{
			// フェードクラスの破棄
			DeleteGO(m_fade);

			return;
		}

		/**
		 * @brief パラメータの更新
		*/
		void CRenderingEngine::ParametersUpdate()
		{
			// ビューカリング用のビュープロジェクション行列の計算。
			CalcViewProjectionMatrixForViewCulling();
			// シーンのジオメトリ情報の更新。
			m_sceneGeometryData.Update();

			return;
		}


		/**
		 * @brief レンダリングエンジンを実行
		 * @param[in] stopWatch ストップウォッチ
		*/
		void CRenderingEngine::Execute(const nsTimer::CStopWatch& stopWatch)
		{
			// レンダリングコンテキスト
			RenderContext& rc = g_graphicsEngine->GetRenderContext();

			// 描画オブジェクトの登録
			GameObjectManager::GetInstance()->ExecuteAddRender();

			// パラメータの更新
			ParametersUpdate();

			// シャドウマップに描画する
			RenderToShadowMap(rc);

			// GBufferに描画する
			RenderToGBuffer(rc);

			// ディファ―ドライディング
			DefferdLighting(rc);

			// フォワードレンダリング
			// フォワードレンダリングの中で
			// rc.WaitUntilFinishDrawingToRenderTarget(m_mainRenderTarget)を呼ばないで
			// エフェクトも一緒にメインレンダリングターゲットに描画してから描画終了待ち処理を呼ぶ
			ForwardRendering(rc);
			// エフェクトの描画
			EffectEngine::GetInstance()->Draw();
			// メインレンダリングターゲットへの書き込み終了待ち
			rc.WaitUntilFinishDrawingToRenderTarget(m_mainRenderTarget);


			// ポストエフェクトを実行
			// ポストエフェクトの中で
			// rc.WaitUntilFinishDrawingToRenderTarget(m_mainRenderTarget)を呼ばないで
			// 2DとFPSも一緒にメインレンダリングターゲットに描画してから描画終了待ち処理を呼ぶ
			m_postEffect.Render(rc, m_mainRenderTarget);
			// 2Dを描画する
			Render2D(rc);
#ifdef MY_DEBUG
			//FPSを描画する
			nsTimer::GameTime().DrawFPS(
				g_graphicsEngine->GetRenderContext(),
				static_cast<float>(stopWatch.GetElapsed())
			);
#endif
			// メインレンダリングターゲットへの書き込み終了待ち
			rc.WaitUntilFinishDrawingToRenderTarget(m_mainRenderTarget);


			// メインレンダリングターゲットの内容をフレームバッファにコピーする
			CopyMainRenderTargetToFrameBuffer(rc);

			m_renderObjects.clear();

			return;
		}

		/**
		 * @brief シャドウマップに描画する
		 * @param[in] rc レンダリングコンテキスト
		*/
		void CRenderingEngine::RenderToShadowMap(RenderContext& rc)
		{
			if (m_sceneGeometryData.IsBuildshadowCasterGeometryData() == false)
			{
				// シャドウキャスターのジオメトリ情報が構築できていなかったら、早期リターン。
				return;
			}

			// 現在のディレクションライトの数
			const int ligNum = nsLight::CLightManager::GetInstance()->GetLightData().directionalLightNum;

			// ライトの数だけ繰り返し
			for (int ligNo = 0; ligNo < ligNum; ligNo++)
			{
				// 該当のディレクションライトの方向
				Vector3& ligDir = nsLight::CLightManager::
					GetInstance()->GetLightData().directionalLightData[ligNo].direction;

				// 該当のディレクションライトが生成するシャドウマップを描画
				m_shadowMapRenders[ligNo].Render(
					rc,
					ligNo,
					ligDir,
					m_renderObjects,
					m_sceneGeometryData.GetShadowCasterMaxPositionInViewFrustum(),
					m_sceneGeometryData.GetShadowCasterMinPositionInViewFrustum()
					);
			}

			return;
		}

		/**
		 * @brief GBufferに描画する
		 * @param rc レンダリングコンテキスト
		*/
		void CRenderingEngine::RenderToGBuffer(RenderContext& rc)
		{
			// レンダリングターゲットをG-Bufferに変更
			RenderTarget* rts[enGBufferNum] = {
				&m_GBuffer[enGBufferAlbedoDepth],         // 0番目のレンダリングターゲット
				&m_GBuffer[enGBufferNormal],              // 1番目のレンダリングターゲット
				&m_GBuffer[enGBufferMetaricShadowSmooth], // 2番目のレンダリングターゲット
			};

			// まず、レンダリングターゲットとして設定できるようになるまで待つ
			rc.WaitUntilToPossibleSetRenderTargets(ARRAYSIZE(rts), rts);

			// レンダリングターゲットを設定
			rc.SetRenderTargetsAndViewport(ARRAYSIZE(rts), rts);

			// レンダリングターゲットをクリア
			rc.ClearRenderTargetViews(ARRAYSIZE(rts), rts);

			// 描画
			for (nsGraphic::CRender* renderObject : m_renderObjects)
			{
				renderObject->OnRenderToGBuffer(rc);
			}

			// レンダリングターゲットへの書き込み待ち
			rc.WaitUntilFinishDrawingToRenderTargets(ARRAYSIZE(rts), rts);

			return;
		}

		/**
		 * @brief ディファ―ドライティング
		 * @param rc レンダリングコンテキスト
		*/
		void CRenderingEngine::DefferdLighting(RenderContext& rc)
		{
			// ディファードライティングに必要なデータを更新する

			// ビュープロジェクション行列の逆行列更新
			m_defferdLightingCB.mViewProjInv.Inverse(g_camera3D->GetViewProjectionMatrix());



			// ライトビュープロジェクション行列を更新
			for (int ligNo = 0; ligNo < nsLight::nsLightConstData::kMaxDirectionalLightNum; ligNo++)
			{
				for (int areaNo = 0; areaNo < nsGraphic::nsShadow::nsShadowConstData::enShadowMapArea_num; areaNo++)
				{
					nsLight::CLightManager::GetInstance()->SetLVPMatrix(
						ligNo,
						areaNo,
						m_shadowMapRenders[ligNo].GetLVPMatrix(areaNo)
					);
				}

				nsLight::CLightManager::GetInstance()->SetPlayerLVPMatrix(
					ligNo,
					m_shadowMapRenders[ligNo].GetPlayerLVPMatrix()
				);
			}

			// レンダリング先をメインレンダリングターゲットにする
			// メインレンダリングターゲットを設定
			rc.WaitUntilToPossibleSetRenderTarget(m_mainRenderTarget);
			rc.SetRenderTargetAndViewport(m_mainRenderTarget);

			// G-Bufferの内容を元にしてディファードライティング
			m_diferredLightingSprite->Draw(rc);

			// メインレンダリングターゲットへの書き込み終了待ち
			rc.WaitUntilFinishDrawingToRenderTarget(m_mainRenderTarget);

			return;
		}

		/**
		 * @brief フォワードレンダリング
		 * @param[in] rc レンダリングコンテキスト
		*/
		void CRenderingEngine::ForwardRendering(RenderContext& rc)
		{
			// レンダリング先をメインレンダリングターゲットにする
			// 深度バッファはGBufferのものを使用する
			rc.WaitUntilToPossibleSetRenderTarget(m_mainRenderTarget);
			rc.SetRenderTarget(
				m_mainRenderTarget.GetRTVCpuDescriptorHandle(),
				m_GBuffer[enGBufferAlbedoDepth].GetDSVCpuDescriptorHandle()
			);

			// 描画
			for (nsGraphic::CRender* renderObject : m_renderObjects)
			{
				renderObject->OnForwardRender(rc);
			}

			// メインレンダリングターゲットへの書き込み終了待ち
			//rc.WaitUntilFinishDrawingToRenderTarget(m_mainRenderTarget);

			return;
		}

		/**
		 * @brief 2Dを描画する
		 * @param rc レンダリングコンテキスト
		*/
		void CRenderingEngine::Render2D(RenderContext& rc)
		{
			for (nsGraphic::CRender* renderObject : m_renderObjects)
			{
				renderObject->OnRender2D(rc);
			}
			return;
		}

		/**
		 * @brief メインレンダリングターゲットの内容をフレームバッファにコピーする
		 * @param rc レンダリングコンテキスト
		*/
		void CRenderingEngine::CopyMainRenderTargetToFrameBuffer(RenderContext& rc)
		{
			// メインレンダリングターゲットの絵をフレームバッファーにコピー
			rc.SetRenderTarget(
				g_graphicsEngine->GetCurrentFrameBuffuerRTV(),
				g_graphicsEngine->GetCurrentFrameBuffuerDSV()
			);

			// ビューポートを指定する
			D3D12_VIEWPORT viewport;
			viewport.TopLeftX = 0;
			viewport.TopLeftY = 0;
			viewport.Width = static_cast<FLOAT>(g_graphicsEngine->GetFrameBufferWidth());
			viewport.Height = static_cast<FLOAT>(g_graphicsEngine->GetFrameBufferHeight());
			viewport.MinDepth = 0.0f;
			viewport.MaxDepth = 1.0f;

			rc.SetViewportAndScissor(viewport);
			m_copyMainRtToFrameBufferSprite.Draw(rc);

			return;
		}

		/**
		 * @brief IBLのデータを初期化する
		 * @param[in] ddsFilePath IBLのテクスチャのddsファイルパス
		 * @param[in] luminance IBLの明るさ
		*/
		void CRenderingEngine::InitIBLData(const wchar_t* ddsFilePath, const float luminance)
		{
			// IBLに使用するテクスチャの初期化
			m_IBLTexture.InitFromDDSFile(ddsFilePath);

			// IBLの明るさを設定
			m_IBLCB.IBLLuminance = luminance;

			return;
		}
	}
}