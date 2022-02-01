/*!
 * @brief シャドウマップ描画用のシェーダー
 */

#include "ModelVSCommon.h"
 // 定義してあればEVSMを行う。PBRLighting.hにも存在している。
#define EVSM

cbuffer ShadowModelCb : register (b1)
{
    float3 playerPos;
}

// ピクセルシェーダーへの入力
struct SPSIn
{
    float4 pos : SV_POSITION;   // スクリーン空間でのピクセルの座標
    //float2 depth : TEXCOORD1;   // ライト空間での深度情報
};

static const int INFINITY = 40.0f;

///////////////////////////////////////////////////
// グローバル変数
///////////////////////////////////////////////////

// モデル用の頂点シェーダーのエントリーポイント
SPSIn VSMainCore(SVSIn vsIn, float4x4 mWorldLocal)
{
    SPSIn psIn;

    psIn.pos = mul(mWorldLocal, vsIn.pos);
    //psIn.pos.xyz -= playerPos;
    float3 worldPos = psIn.pos;
    psIn.pos = mul(mView, psIn.pos);
    psIn.pos = mul(mProj, psIn.pos);

    return psIn;
}
/// <summary>
/// スキンなしメッシュ用の頂点シェーダーのエントリー関数。
/// </summary>
SPSIn VSMain(SVSIn vsIn)
{
	return VSMainCore(vsIn, mWorld);
}

/// <summary>
/// スキンありメッシュの頂点シェーダーのエントリー関数。
/// </summary>
SPSIn VSSkinMain( SVSIn vsIn )
{
	return VSMainCore(vsIn, CalcSkinMatrix(vsIn));
}
SPSIn VSMainInstancing(SVSIn vsIn, uint instanceID : SV_InstanceID)
{
    return VSMainCore(vsIn, g_worldMatrixArray[instanceID]);
}
SPSIn VSMainSkinInstancing(SVSIn vsIn, uint instanceID : SV_InstanceID)
{
    float4x4 mWorldLocal = CalcSkinMatrix(vsIn);
    mWorldLocal = mul(g_worldMatrixArray[instanceID], mWorldLocal);
    return VSMainCore(vsIn, mWorldLocal);
}

/// <summary>
/// シャドウマップ描画用のピクセルシェーダー
/// </summary>
float4 PSMain(SPSIn psIn) : SV_Target0
{
    // 旧バージョン
#ifndef EVSM
     float depth = psIn.pos.z;
     return float4(depth, depth * depth, 0.0f, 1.0f);
#else

    // EVSM
    float depth = psIn.pos.z;
    float pos = exp(INFINITY * depth);
    return float4(pos, pos * pos, 0.0f, 1.0f);
#endif
}
