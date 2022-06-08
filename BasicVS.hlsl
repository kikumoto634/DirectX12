#include "Basic.hlsli"

VSOutput main( float4 pos : POSITION , float3 normal : NORMAL, float2 uv : TEXCOORD)
{
	VSOutput output;	//�s�N�Z���V�F�[�_�[�ɓn���l
	//Scale
	pos = pos * float4(1.0f, 1.0f, 1.0f, 1);
	//Trans
	pos = pos + float4(0.0f, 0.0f, 0.0f, 0);

	output.svpos = mul(mat,pos);
	output.normal = normal;
	output.uv = uv;

	return output;
}