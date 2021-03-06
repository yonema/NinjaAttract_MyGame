#include "stdafx.h"
#include "Sprite.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "../GameTemplate/Game/MyEngine.h"
#include "../GameTemplate/Game/RenderingEngine.h"

/**
 * @brief コンストラクタ
*/
SpriteInitData::SpriteInitData()
{
	// ユーザー拡張用の定数バッファを初期化
	for (int i = 0; i < m_kMaxExCBNum; i++)
	{
		m_expandConstantBuffer[i] = nullptr;
		m_expandConstantBufferSize[i] = 0;
	}
	return;
}

	namespace {
		struct SSimpleVertex {
			Vector4 pos;
			Vector2 tex;
		};
	}
	const Vector2	Sprite::DEFAULT_PIVOT = { 0.5f, 0.5f };
	Sprite::~Sprite()
	{
	}
	void Sprite::InitTextures(const SpriteInitData& initData)
	{
		//スプライトで使用するテクスチャを準備する。
		if (initData.m_ddsFilePath[0] != nullptr) {
			//ddsファイルのパスが指定されてるのなら、ddsファイルからテクスチャを作成する。
			int texNo = 0;
			while (initData.m_ddsFilePath[texNo] && texNo < MAX_TEXTURE) {

				// 変更。

				// テクスチャバンクからリソースを探して取ってくる
				m_textures[texNo] = nsNinjaAttract::nsMyEngine::
					CRenderingEngine::GetInstance()->GetTextureFileFromBank(initData.m_ddsFilePath[texNo]);

				if (m_textures[texNo] == nullptr)
				{
					// リソースがなかったら新しく生成する
					wchar_t wddsFilePath[1024];
					mbstowcs(wddsFilePath, initData.m_ddsFilePath[texNo], 1023);
					m_textures[texNo] = new Texture();
					m_textures[texNo]->InitFromDDSFile(wddsFilePath);
					// リソースを登録する
					nsNinjaAttract::nsMyEngine::
						CRenderingEngine::GetInstance()->RegistTextureFileToBank(initData.m_ddsFilePath[texNo], m_textures[texNo]);
				}

				texNo++;
			}
			m_numTexture = texNo;
		}
		else if (initData.m_textures[0] != nullptr) {
			//外部テクスチャを指定されている。
			int texNo = 0;
			while (initData.m_textures[texNo] != nullptr) {
				m_textureExternal[texNo] = initData.m_textures[texNo];
				texNo++;
			}
			m_numTexture = texNo;
		}
		else {
			//テクスチャが指定されてない。
			MessageBoxA(nullptr, "initData.m_ddsFilePathかm_texturesのどちらかに使用するテクスチャの情報を設定してください。", "エラー", MB_OK);
			std::abort();
		}
	}
	void Sprite::InitShader(const SpriteInitData& initData)
	{
		if (initData.m_fxFilePath == nullptr) {
			MessageBoxA(nullptr, "fxファイルが指定されていません。", "エラー", MB_OK);
			std::abort();
		}
		wchar_t fxFilePath[1024];
		mbstowcs(fxFilePath, initData.m_fxFilePath, 1023);
		//シェーダーをロードする。
		m_vs.LoadVS(fxFilePath, initData.m_vsEntryPointFunc);
		m_ps.LoadPS(fxFilePath, initData.m_psEntryPoinFunc);
	}
	void Sprite::InitDescriptorHeap(const SpriteInitData& initData)
	{
		if (m_textureExternal[0] != nullptr) {
			//外部のテクスチャが指定されている。
			for (int texNo = 0; texNo < m_numTexture; texNo++) {
				m_descriptorHeap.RegistShaderResource(texNo, *m_textureExternal[texNo]);
			}
		}
		else {
			for (int texNo = 0; texNo < m_numTexture; texNo++) {
				m_descriptorHeap.RegistShaderResource(texNo, *m_textures[texNo]);
			}
		}
		if (initData.m_expandShaderResoruceView != nullptr) {
			//拡張シェーダーリソースビュー。
			m_descriptorHeap.RegistShaderResource(
				EXPAND_SRV_REG__START_NO,
				*initData.m_expandShaderResoruceView
			);
		}
		m_descriptorHeap.RegistConstantBuffer(0, m_constantBufferGPU);

		// 変更。追加。
		for (int i = 0; i < SpriteInitData::m_kMaxExCBNum; i++)
		{
			if (m_userExpandConstantBufferCPU[i] != nullptr) {
				//ユーザー拡張の定数バッファはb1に関連付けする。
				m_descriptorHeap.RegistConstantBuffer(1 + i, m_userExpandConstantBufferGPU[i]);
			}
		}
		m_descriptorHeap.Commit();
	}
	void Sprite::InitVertexBufferAndIndexBuffer(const SpriteInitData& initData)
	{
		float halfW = m_size.x * 0.5f;
		float halfH = m_size.y * 0.5f;
		//頂点バッファのソースデータ。
		SSimpleVertex vertices[] =
		{
			{
				Vector4(-halfW, -halfH, 0.0f, 1.0f),
				Vector2(0.0f, 1.0f),
			},
			{
				Vector4(halfW, -halfH, 0.0f, 1.0f),
				Vector2(1.0f, 1.0f),
			},
			{
				Vector4(-halfW, halfH, 0.0f, 1.0f),
				Vector2(0.0f, 0.0f)
			},
			{
				Vector4(halfW, halfH, 0.0f, 1.0f),
				Vector2(1.0f, 0.0f)
			}

		};
		unsigned short indices[] = { 0,1,2,3 };

		m_vertexBuffer.Init(sizeof(vertices), sizeof(vertices[0]));
		m_vertexBuffer.Copy(vertices);

		m_indexBuffer.Init(sizeof(indices), sizeof(indices[0]));
		m_indexBuffer.Copy(indices);
	}
	void Sprite::InitPipelineState(const SpriteInitData& initData)
	{
		// 頂点レイアウトを定義する。
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		//パイプラインステートを作成。
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = { 0 };
		psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		psoDesc.pRootSignature = m_rootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_vs.GetCompiledBlob());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_ps.GetCompiledBlob());
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

		if (initData.m_alphaBlendMode == AlphaBlendMode_Trans) {
			//半透明合成のブレンドステートを作成する。
			psoDesc.BlendState.RenderTarget[0].BlendEnable = true;
			psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
			psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
			psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		}
		else if (initData.m_alphaBlendMode == AlphaBlendMode_Add) {
			//加算合成。
			psoDesc.BlendState.RenderTarget[0].BlendEnable = true;
			psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
			psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
			psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		}

		psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		psoDesc.DepthStencilState.DepthEnable = FALSE;
		psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
		psoDesc.DepthStencilState.StencilEnable = FALSE;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

		// 変更
		for (auto& format : initData.m_colorBufferFormat) {
			if (format == DXGI_FORMAT_UNKNOWN) {
				break;
			}
			psoDesc.RTVFormats[psoDesc.NumRenderTargets] = format;
			psoDesc.NumRenderTargets++;
		}

		psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		psoDesc.SampleDesc.Count = 1;

		m_pipelineState.Init(psoDesc);
	}
	void Sprite::InitConstantBuffer(const SpriteInitData& initData)
	{
		//定数バッファの初期化。
		m_constantBufferGPU.Init(sizeof(m_constantBufferCPU), nullptr);

		// 変更。追加。
		for (int i = 0; i < SpriteInitData::m_kMaxExCBNum; i++)
		{
			//ユーザー拡張の定数バッファが指定されている。
			if (initData.m_expandConstantBuffer[i] != nullptr)
			{
				m_userExpandConstantBufferCPU[i] = initData.m_expandConstantBuffer[i];
				m_userExpandConstantBufferGPU[i].Init(
					initData.m_expandConstantBufferSize[i],
					initData.m_expandConstantBuffer[i]
				);
			}
		}
	}
	void Sprite::Init(const SpriteInitData& initData)
	{
		m_size.x = static_cast<float>(initData.m_width);
		m_size.y = static_cast<float>(initData.m_height);

		//テクスチャを初期化。
		InitTextures(initData);
		//頂点バッファとインデックスバッファを初期化。
		InitVertexBufferAndIndexBuffer(initData);
		//定数バッファを初期化。
		InitConstantBuffer(initData);
		
		//ルートシグネチャの初期化。
		m_rootSignature.Init(
			D3D12_FILTER_MIN_MAG_MIP_LINEAR,
			initData.textureAddressMode,	// 変更
			initData.textureAddressMode,	// 変更
			initData.textureAddressMode		// 変更
		);

		//シェーダーを初期化。
		InitShader(initData);
		//パイプラインステートの初期化。
		InitPipelineState(initData);
		//ディスクリプタヒープを初期化。
		InitDescriptorHeap(initData);

		m_isInited = true;
	}
	void Sprite::Update(const Vector3& pos, const Quaternion& rot, const Vector3& scale, const Vector2& pivot)
	{
		if (m_isInited == false) {
			//未初期化
			return;
		}
		//ピボットを考慮に入れた平行移動行列を作成。
		//ピボットは真ん中が0.0, 0.0、左上が-1.0f, -1.0、右下が1.0、1.0になるようにする。
		Vector2 localPivot = pivot;
		localPivot.x -= 0.5f;
		localPivot.y -= 0.5f;
		localPivot.x *= -2.0f;
		localPivot.y *= -2.0f;
		//画像のハーフサイズを求める。
		Vector2 halfSize = m_size;
		halfSize.x *= 0.5f;
		halfSize.y *= 0.5f;
		Matrix mPivotTrans;

		mPivotTrans.MakeTranslation(
			{ halfSize.x * localPivot.x, halfSize.y * localPivot.y, 0.0f }
		);
		Matrix mTrans, mRot, mScale;
		mTrans.MakeTranslation(pos);
		mRot.MakeRotationFromQuaternion(rot);
		mScale.MakeScaling(scale);
		m_world = mPivotTrans * mScale;
		m_world = m_world * mRot;
		m_world = m_world * mTrans;
	}
	void Sprite::Draw(RenderContext& renderContext)
	{
		if (m_isInited == false) {
			//未初期化。
			return;
		}
		//現在のビューポートから平行投影行列を計算する。
		D3D12_VIEWPORT viewport = renderContext.GetViewport();

		Matrix viewMatrix = g_camera2D->GetViewMatrix();
		//Matrix projMatrix = g_camera2D->GetProjectionMatrix();
		Matrix projMatrix;
		projMatrix.MakeOrthoProjectionMatrix(viewport.Width, viewport.Height, 0.1f, 1.0f);

		m_constantBufferCPU.mvp = m_world * viewMatrix * projMatrix;
		//m_constantBufferCPU.mulColor.x = 1.0f;
		//m_constantBufferCPU.mulColor.y = 1.0f;
		//m_constantBufferCPU.mulColor.z = 1.0f;
		//m_constantBufferCPU.mulColor.w = 1.0f;
		m_constantBufferCPU.screenParam.x = g_camera3D->GetNear();
		m_constantBufferCPU.screenParam.y = g_camera3D->GetFar();
		m_constantBufferCPU.screenParam.z = FRAME_BUFFER_W;
		m_constantBufferCPU.screenParam.w = FRAME_BUFFER_H;

		//定数バッファを更新。
		m_constantBufferGPU.CopyToVRAM(&m_constantBufferCPU);
		//変更。追加。
		for (int i = 0; i < SpriteInitData::m_kMaxExCBNum; i++)
		{
			if (m_userExpandConstantBufferCPU[i] != nullptr)
			{
				m_userExpandConstantBufferGPU[i].CopyToVRAM(m_userExpandConstantBufferCPU[i]);
			}
		}
		//ルートシグネチャを設定。
		renderContext.SetRootSignature(m_rootSignature);
		//パイプラインステートを設定。
		renderContext.SetPipelineState(m_pipelineState);
		//頂点バッファを設定。
		renderContext.SetVertexBuffer(m_vertexBuffer);
		//インデックスバッファを設定。
		renderContext.SetIndexBuffer(m_indexBuffer);
		//プリミティブトポロジーを設定する。
		renderContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		//ディスクリプタヒープを設定する。
		renderContext.SetDescriptorHeap(m_descriptorHeap);
		//描画
		renderContext.DrawIndexed(m_indexBuffer.GetCount());
	}


	// 追加
	Sprite::Sprite()
	{
		m_constantBufferCPU.mulColor.x = 1.0f;
		m_constantBufferCPU.mulColor.y = 1.0f;
		m_constantBufferCPU.mulColor.z = 1.0f;
		m_constantBufferCPU.mulColor.w = 1.0f;

		return;
	}
