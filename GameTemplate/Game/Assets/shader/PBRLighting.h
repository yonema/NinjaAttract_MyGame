///////////////////////////////////////
// 定数
///////////////////////////////////////
static const float PI = 3.1415926f;         // π

//static const int MAX_POINT_LIGHT = 1000;    // ポイントライトの最大数

//#define TILE_WIDTH 16
//#define TILE_HEIGHT 16
static const int INFINITY = 40.0f;

// 定義してあればEVSMを行う。DrawShadowMap.fxにも存在している。
#define EVSM

///////////////////////////////////////
// 構造体。
///////////////////////////////////////

//lightData.hと同じ値。変更したら一緒に変更する。
// ディレクションライト構造体。
struct SDirectionalLightData
{
    float3 direction;   // ライトの方向
    int castShadow;     // 影をキャストする？
    float4 color;       // ライトの色
};
// ポイントライト
struct SPointLightData
{
    float3 position;        // 座標
    float range;            // 影響範囲
    float3 color;           // カラー
};
// スポットライト
struct SSpotLightData
{
    SPointLightData pointLightData;				//!< ポイントライトのデータ
    float3 direction;	//!< ライトの射出方向
    float angle;		//!< ライトの射出角度
};


///////////////////////////////////////
// 定数バッファ。
///////////////////////////////////////

//lightData.hと同じ値。変更したら一緒に変更する。
// ライト用の定数バッファー
cbuffer LightCb : register(b1)
{
    SDirectionalLightData directionalLightData[kMaxDirectionalLightNum];	//!< ディレクションライトのデータ
    SPointLightData pointLightData[kMaxPointLightNum];	//!< ポイントライトのデータ
    SSpotLightData spotLightData[kMaxSpotLightNum];		//!< スポットライトのデータ
    float3 eyePos;				//!< 視点
    int directionalLightNum;    //!< ディレクションライトの数
    float3 ambientLight;	    //!< アンビエントライト
    int pointLightNum;			//!< ポイントライトの数
    //!< ライトビュープロジェクション
    float4x4 mlvp[kMaxDirectionalLightNum][kMaxShadowMapNum];
    //!< プレイヤー専用のライトビュープロジェクション
    float4x4 mPlayerLvp[kMaxDirectionalLightNum];
    int spotLightNum;			//!< スポットライトの数
};

///////////////////////////////////////
// テクスチャ
///////////////////////////////////////


///////////////////////////////////////
// サンプラステート。
///////////////////////////////////////
sampler g_sampler : register(s0);


///////////////////////////////////////
// 関数
///////////////////////////////////////

// 旧バージョン
float VarianceShadowMap(float2 shadowValue, float zInLVP)
{
    // 分散シャドウマップ
    // 遮蔽されているなら、チェビシェフの不等式を利用して光が当たる確率を求める
    float depth_sq = shadowValue.x * shadowValue.x;
    // このグループの分散具合を求める
    // 分散が大きいほど、varianceの数値は大きくなる
    float variance = min(max(shadowValue.y - depth_sq, 0.0001f), 1.0f);
    // このピクセルのライトから見た深度値とシャドウマップの平均の深度値の差を求める
    float md = zInLVP - shadowValue.x;
    // 光が届く確率を求める
    float lit_factor = variance / (variance + md * md);
    // 光が届く確率から影になる確率を計算する。
    float shadow = 1.0f - pow(lit_factor, 5.0f);
    //shadow = 1.0f;

    return shadow;
}

// チェビシェフの不等式を利用して、影になる可能性を計算する。
float Chebyshev(float2 moments, float depth)
{
    if (depth <= moments.x) {
        return 0.0;
    }
    // 遮蔽されているなら、チェビシェフの不等式を利用して光が当たる確率を求める
    float depth_sq = moments.x * moments.x;
    // このグループの分散具合を求める
    // 分散が大きいほど、varianceの数値は大きくなる
    float variance = moments.y - depth_sq;
    // このピクセルのライトから見た深度値とシャドウマップの平均の深度値の差を求める
    float md = depth - moments.x;
    // 光が届く確率を求める
    float lit_factor = variance / (variance + md * md);
    float lig_factor_min = 0.3f;
    // 光が届く確率の下限以下は影になるようにする。
    lit_factor = saturate((lit_factor - lig_factor_min) / (1.0f - lig_factor_min));
    // 光が届く確率から影になる確率を求める。
    return 1.0f - lit_factor;
}

float CalcShadowRate(int ligNo, float3 worldPos)
{
    float shadow = 0.0f;
    for(int cascadeIndex = 0; cascadeIndex < kMaxShadowMapNum; cascadeIndex++)
    {
        float4 posInLVP = mul( mlvp[ligNo][cascadeIndex], float4( worldPos, 1.0f ));
        float2 shadowMapUV = posInLVP.xy / posInLVP.w;
        float zInLVP = posInLVP.z / posInLVP.w;

        // ライトカメラの遠平面より遠かったら、影を落とさない

        if (zInLVP >= 1.0f)
        {
            return shadow;
        }


        shadowMapUV *= float2(0.5f, -0.5f);
        shadowMapUV += 0.5f;

        // シャドウマップUVが範囲内か判定
        if(shadowMapUV.x >= 0.0f && shadowMapUV.x <= 1.0f
            && shadowMapUV.y >= 0.0f && shadowMapUV.y <= 1.0f)
        {
            // シャドウマップから値をサンプリング
            float2 shadowValue = g_shadowMap[ligNo][cascadeIndex].Sample(g_sampler, shadowMapUV).xy;
            if (shadowValue.r < 0.0f)
                break;

            // EVSM
#ifdef EVSM
            zInLVP -= 0.001f;
            float pos = exp(INFINITY * zInLVP);
            shadow = Chebyshev(shadowValue.xy, pos);

            break;
#endif

            // 旧バージョン

            // まずこのピクセルが遮蔽されているか調べる
            if (zInLVP >= shadowValue.r/* + 0.0001f*/)
            {
                // ソフトシャドウ
                shadow = VarianceShadowMap(shadowValue, zInLVP);

                // ライトカメラ（光源）からの距離に応じて影を指数関数的に薄くしていく。
                shadow *= (1.0f - pow(zInLVP, 2.0f));
                // ライトカメラ（光源）からの距離に応じて影を線形に薄くしていく。
                //shadow *= (1.0f - zInLVP);
            }
            
            break;
        }
    }
    return shadow;
}

float CalcPlayerShadowRate(int ligNo, float3 worldPos)
{
    float shadow = 0.0f;
    float4 posInLVP = mul(mPlayerLvp[ligNo], float4(worldPos, 1.0f));
    float2 shadowMapUV = posInLVP.xy / posInLVP.w;
    float zInLVP = posInLVP.z / posInLVP.w;

    // ライトカメラの遠平面より遠かったら、影を落とさない
    if (zInLVP >= 1.0f)
    {
        return shadow;
    }

    shadowMapUV *= float2(0.5f, -0.5f);
    shadowMapUV += 0.5f;
    // シャドウマップUVが範囲内か判定
    if (shadowMapUV.x >= 0.0f && shadowMapUV.x <= 1.0f
        && shadowMapUV.y >= 0.0f && shadowMapUV.y <= 1.0f)
    {
        // シャドウマップから値をサンプリング
        float2 shadowValue = g_playerShadowMap[ligNo].Sample(g_sampler, shadowMapUV).xy;
        if (shadowValue.r < 0.0f)
            return shadow;

        // EVSM
#ifdef EVSM
        zInLVP -= 0.001f;
        float pos = exp(INFINITY * zInLVP);
        shadow = Chebyshev(shadowValue.xy, pos);

        // ライトカメラ（光源）からの距離に応じて影を指数関数的に薄くしていく。
        shadow *= (1.0f - pow(zInLVP, 2.0f));

        return shadow;
#endif

        // 旧バージョン
        // まずこのピクセルが遮蔽されているか調べる
        if (zInLVP >= shadowValue.r + 0.0001f)
        {
            // ソフトシャドウ
            shadow = VarianceShadowMap(shadowValue, zInLVP);

            // ライトカメラ（光源）からの距離に応じて影を指数関数的に薄くしていく。
            shadow *= (1.0f - pow(zInLVP,2.0f));
            // ライトカメラ（光源）からの距離に応じて影を線形に薄くしていく。
            //shadow *= (1.0f - zInLVP);
        }

    }

    return shadow;
}



// ベックマン分布を計算する
float Beckmann(float m, float t)
{
    float t2 = t * t;
    float t4 = t * t * t * t;
    float m2 = m * m;
    float D = 1.0f / (4.0f * m2 * t4);
    D *= exp((-1.0f / m2) * (1.0f-t2)/ t2);
    return D;
}

// フレネルを計算。Schlick近似を使用
float SpcFresnel(float f0, float u)
{
    // from Schlick
    return f0 + (1-f0) * pow(1-u, 5);
}

/// <summary>
/// クックトランスモデルの鏡面反射を計算
/// </summary>
/// <param name="L">光源に向かうベクトル</param>
/// <param name="V">視点に向かうベクトル</param>
/// <param name="N">法線ベクトル</param>
/// <param name="smooth">滑らかさ</param>
float CookTorranceSpecular(float3 L, float3 V, float3 N, float smooth, float metaric)
{
    float microfacet = 1.0f - smooth;

    // 金属度を垂直入射の時のフレネル反射率として扱う
    // 金属度が高いほどフレネル反射は大きくなる
    float f0 = metaric;

    // ライトに向かうベクトルと視線に向かうベクトルのハーフベクトルを求める
    float3 H = normalize(L + V);

    // 各種ベクトルがどれくらい似ているかを内積を利用して求める
    float NdotH = max( saturate(dot(N, H)), 0.001f );
    float VdotH = max( saturate(dot(V, H)), 0.001f );
    float NdotL = max( saturate(dot(N, L)), 0.001f );
    float NdotV = max( saturate(dot(N, V)), 0.001f );

    // D項をベックマン分布を用いて計算する
    float D = Beckmann(microfacet, NdotH);

    // F項をSchlick近似を用いて計算する
    float F = SpcFresnel(f0, VdotH);

    // G項を求める
    float G = min(1.0f, min(2*NdotH*NdotV/VdotH, 2*NdotH*NdotL/VdotH));

    // m項を求める
    float m = PI * NdotV * NdotH;

    // ここまで求めた、値を利用して、クックトランスモデルの鏡面反射を求める
    return max(F * D * G / m, 0.0);
}

/// <summary>
/// フレネル反射を考慮した拡散反射を計算
/// </summary>
/// <remark>
/// この関数はフレネル反射を考慮した拡散反射率を計算します
/// フレネル反射は、光が物体の表面で反射する現象のとこで、鏡面反射の強さになります
/// 一方拡散反射は、光が物体の内部に入って、内部錯乱を起こして、拡散して反射してきた光のことです
/// つまりフレネル反射が弱いときには、拡散反射が大きくなり、フレネル反射が強いときは、拡散反射が小さくなります
///
/// </remark>
/// <param name="N">法線</param>
/// <param name="L">光源に向かうベクトル。光の方向と逆向きのベクトル。</param>
/// <param name="V">視線に向かうベクトル。</param>
/// <param name="roughness">粗さ。0〜1の範囲。</param>
float CalcDiffuseFromFresnel(float3 N, float3 L, float3 V, float smooth)
{
    // step-1 ディズニーベースのフレネル反射による拡散反射を真面目に実装する。
    // 光源に向かうベクトルと視線に向かうベクトルのハーフベクトルを求める
    float3 H = normalize(L+V);
    
    //粗さ
    float roughness = 1.0f - smooth;
    
    //これはEA DICEが発表した改良コード。
    float energyBias = lerp(0.0f, 0.5f, roughness);
    float energyFactor = lerp(1.0f, 1.0f / 1.51f, roughness);

    // 光源に向かうベクトルとハーフベクトルがどれだけ似ているかを内積で求める
    float dotLH = saturate(dot(L,H));
    // 光源に向かうベクトルとハーフベクトル、光が平行に入射したときの拡散反射量を求めている。
    float Fd90 = energyBias + 2.0f * dotLH * dotLH * roughness;
    
    // 法線と光源に向かうベクトルｗを利用して拡散反射率を求めています
    float dotNL = saturate(dot(N,L));
    float FL = (1.0f + (Fd90 - 1.0f) * pow(1.0f - dotNL, 5.0f));

    
    // 法線と視点に向かうベクトルを利用して拡散反射率を求めています
    float dotNV = saturate(dot(N,V));
    float FV =  (1.0f + (Fd90 - 1.0f) * pow(1.0f - dotNV, 5.0f));

    //法線と光源への方向に依存する拡散反射率と、法線と視点ベクトルに依存する拡散反射率を
    // 乗算して最終的な拡散反射率を求めている。PIで除算しているのは正規化を行うため
    return (FL*FV * energyFactor);
}



// 反射光を計算する。
float3 CalcLighting(
    float3 ligDir, 
    float3 ligColor, 
    float3 normal,
    float3 toEye, 
    float4 albedoColor,  
    float metaric, 
    float smooth, 
    float3 specColor
    )
{
    // 影が落ちていないのでライトの計算を行う。
    // ディズニーベースの拡散反射を実装する
    // フレネル反射を考慮した拡散反射を計算
    float diffuseFromFresnel = CalcDiffuseFromFresnel(
        normal, -ligDir, toEye, smooth);

    // 正規化Lambert拡散反射を求める
    float NdotL = saturate(dot(normal, -ligDir));
    float3 lambertDiffuse = ligColor * NdotL / PI;

    // 最終的な拡散反射光を計算する
    float3 diffuse = albedoColor * diffuseFromFresnel * lambertDiffuse;

    // クックトランスモデルを利用した鏡面反射率を計算する
    // クックトランスモデルの鏡面反射率を計算する
    float3 spec = CookTorranceSpecular(
        -ligDir, toEye, normal, smooth, metaric)
        * ligColor;

    // 金属度が高ければ、鏡面反射はスペキュラカラー、低ければ白
    // スペキュラカラーの強さを鏡面反射率として扱う

    spec *= lerp(float3(1.0f, 1.0f, 1.0f), specColor, metaric);

    // 滑らかさを使って、拡散反射光と鏡面反射光を合成する
    return max( float3( 0.0f, 0.0f, 0.0f ), diffuse * (1.0f - smooth) + spec * smooth );   
}

