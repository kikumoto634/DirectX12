cbuffer ConstBufferDataMaterial : register(b0)
{
	float4 color;	//�F(RGBA)�@�萔�o�b�t�@���
};

float4 main() : SV_TARGET
{
	return color;
}