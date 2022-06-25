
//マテリアル
cbuffer ConstBufferData : register(b0)
{
	float4 color;	//色(RGBA)　定数バッファ情報
	matrix mat;
};

//頂点シェーダーの出力構造体
//(頂点シェーダーからピクセルシェーダーへのより取りに使用する)
struct VSOutput
{
	//システム用頂点座標
	float4 svpos : SV_POSITION;
	//uv値
	float2 uv : TEXCOORD;
};