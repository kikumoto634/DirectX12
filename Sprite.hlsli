
//3D�ϊ��s��
cbuffer ConstBufferData : register(b0)
{
	matrix mat;	//3D�ϊ��s��
	float4 color;	//�F(RGBA)�@�萔�o�b�t�@���
};

//���_�V�F�[�_�[�̏o�͍\����
//(���_�V�F�[�_�[����s�N�Z���V�F�[�_�[�ւ̂����Ɏg�p����)
struct VSOutput
{
	//�V�X�e���p���_���W
	float4 svpos : SV_POSITION;
	//uv�l
	float2 uv : TEXCOORD;
};