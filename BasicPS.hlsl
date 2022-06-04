cbuffer ConstBufferDataMaterial : register(b0)
{
	float4 color;	//色(RGBA)　定数バッファ情報
};

float4 main() : SV_TARGET
{
	return color;
}