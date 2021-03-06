#pragma once

#include "tkFile/TkmFile.h"
#include "MeshParts.h"
#include "Skeleton.h"

class IShaderResource;

//モデルの上方向
enum EnModelUpAxis {
	enModelUpAxisY,		//モデルの上方向がY軸。
	enModelUpAxisZ,		//モデルの上方向がZ軸。
};
/// <summary>
/// モデルの初期化データ
/// </summary>
struct ModelInitData {
	const char* m_tkmFilePath = nullptr;							//tkmファイルパス。
	const char* m_vsEntryPointFunc = "VSMain";						//頂点シェーダーのエントリーポイント。
	const char* m_vsSkinEntryPointFunc = "VSMain";					//スキンありマテリアル用の頂点シェーダーのエントリーポイント。
	const char* m_psEntryPointFunc = "PSMain";						//ピクセルシェーダーのエントリーポイント。
	const char* m_fxFilePath = nullptr;								//.fxファイルのファイルパス。
	void* m_expandConstantBuffer[MeshParts::m_kMaxExCBNum] = {};	//ユーザー拡張の定数バッファ。
	int m_expandConstantBufferSize[MeshParts::m_kMaxExCBNum] = {};	//ユーザー拡張の定数バッファのサイズ。
	IShaderResource* m_expandShaderResoruceView[MeshParts::m_kMaxExSRVNum] = {};	//ユーザー拡張のシェーダーリソース。
	Skeleton* m_skeleton = nullptr;									//スケルトン。
	EnModelUpAxis m_modelUpAxis = enModelUpAxisZ;					//モデルの上方向。

	// 変更。追加。
	D3D12_CULL_MODE m_cullMode = D3D12_CULL_MODE_BACK;
	const char* m_lodTkmFilePath = nullptr;	// LOD用のtkmファイルパス
	int m_lodNum = 0;						// LODの番号

	// <カラーバッファーフォーマット, レンダリングターゲットの最大数>
	std::array<DXGI_FORMAT, D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT> m_colorBufferFormat = {
	DXGI_FORMAT_R8G8B8A8_UNORM,
	DXGI_FORMAT_UNKNOWN,
	DXGI_FORMAT_UNKNOWN,
	DXGI_FORMAT_UNKNOWN,
	DXGI_FORMAT_UNKNOWN,
	DXGI_FORMAT_UNKNOWN,
	DXGI_FORMAT_UNKNOWN,
	DXGI_FORMAT_UNKNOWN,
	};	//レンダリングするカラーバッファのフォーマット。
public:
	/**
	 * @brief コンストラクタ
	*/
	ModelInitData();
};


/// <summary>
/// モデルクラス。
/// </summary>
class Model {

public:

	/// <summary>
	/// tkmファイルから初期化。
	/// </summary>
	/// <param name="initData">初期化データ</param>
	void Init( const ModelInitData& initData );
	/// <summary>
	/// ワールド行列の更新。
	/// </summary>
	/// <param name="pos">座標</param>
	/// <param name="rot">回転</param>
	/// <param name="scale">拡大率</param>
	void UpdateWorldMatrix(Vector3 pos, Quaternion rot, Vector3 scale);

	/**
	 * @brief 描画
	 * @param[in] renderContext レンダリングコンテキスト
	 * @param[in] numInstance インスタンス数
	*/
	void Draw(RenderContext& renderContext, const int numInstance = 1);
	/// <summary>
	/// ワールド行列を取得。
	/// </summary>
	/// <returns></returns>
	const Matrix& GetWorldMatrix() const
	{
		return m_world;
	}
	/// <summary>
	/// メッシュに対して問い合わせを行う。
	/// </summary>
	/// <param name="queryFunc">問い合わせ関数</param>
	void QueryMeshs(std::function<void(const SMesh& mesh)> queryFunc) 
	{
		m_meshParts.QueryMeshs(queryFunc);
	}
	void QueryMeshAndDescriptorHeap(std::function<void(const SMesh& mesh, const DescriptorHeap& ds)> queryFunc)
	{
		m_meshParts.QueryMeshAndDescriptorHeap(queryFunc);
	}
	/// <summary>
	/// アルベドマップを変更。
	/// </summary>
	/// <remarks>
	/// この関数を呼び出すとディスクリプタヒープの再構築が行われるため、処理負荷がかかります。
	/// 毎フレーム呼び出す必要がない場合は呼び出さないようにしてください。
	/// </remarks>
	/// <param name="materialName">変更しいたマテリアルの名前</param>
	/// <param name="albedoMap">アルベドマップ</param>
	void ChangeAlbedoMap(const char* materialName, Texture& albedoMap);
	/// <summary>
	/// TKMファイルを取得。
	/// </summary>
	/// <returns></returns>
	const TkmFile* GetTkmFile() const
	{
		return m_tkmFile;
	}
private:

	Matrix m_world;														//ワールド行列。
	// 変更。追加。
	TkmFile* m_tkmFile = nullptr;										//tkmファイル。
	Skeleton m_skeleton;												//スケルトン。
	MeshParts m_meshParts;											//メッシュパーツ。
	EnModelUpAxis m_modelUpAxis = enModelUpAxisY;		//モデルの上方向。

	// 追加
public:		// メンバ関数

	/**
	 * @brief 行列を指定して描画
	 * @param[in] rc レンダリングコンテキスト
	 * @param[in] viewMatrix ビュー行列
	 * @param[in] projMatrix プロジェクション行列
	*/
	void Draw(
		RenderContext& rc,
		const Matrix& viewMatrix,
		const Matrix& projMatrix,
		const int numInstance = 1
	);

	/**
	 * @brief 自己発光カラーを設定
	 * @param[in] emmisonColor 自己発光カラー
	*/
	void SetEmmisonColor(const Vector4& emmisonColor)
	{
		m_emmisonColor = emmisonColor;
	}

	/**
	 * @brief 乗算カラーを設定
	 * @param[in] mulColor 乗算カラー
	*/
	void SetMulColor(const Vector4& mulColor)
	{
		m_mulColor = mulColor;
	}

	/**
	 * @brief モデルのアルファ値を設定
	 * @param[in] alphaValue アルファ値
	*/
	void SetAlphaValue(const float alphaValue)
	{
		m_mulColor.w = alphaValue;
	}

	/**
	 * @brief Modelクラスの設定に基づいたワールド行列を計算し、計算されたワールド行列が戻り値として返す。
	 * @param[in] pos 座標
	 * @param[in] rot 回転
	 * @param[in] scale 拡大率
	 * @return ワールド行列
	*/
	Matrix CalcWorldMatrix(const Vector3& pos, const Quaternion& rot, const Vector3& scale) const;

private:	// データメンバ
	Vector4 m_emmisonColor = Vector4::Zero;		//!< 自己発光カラー
	Vector4 m_mulColor = Vector4::One;			//!< 乗算カラー

};
