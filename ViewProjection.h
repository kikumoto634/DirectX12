#pragma once

#include <DirectXMath.h>
#include <d3d12.h>
#include <wrl.h>

///�萔�o�b�t�@�p
struct ConstBufferDataViewProjection
{
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX projection;
	DirectX::XMFLOAT3 cameraPos;
};

/// <summary>
/// �r���[�v���W�F�N�V����
/// </summary>
struct ViewProjection
{
	//�萔�o�b�t�@
	Microsoft::WRL::ComPtr<ID3D12Resource> constBuff;
	//�}�b�s���O
	ConstBufferDataViewProjection* constMap = nullptr;

#pragma region �r���[�s��
	//���_���W
	DirectX::XMFLOAT3 eye = {0, 0, -50.f};
	//�����_���W
	DirectX::XMFLOAT3 target = {0,0,0};
	//������x�N�g��
	DirectX::XMFLOAT3 up = {0, 1, 0};
#pragma endregion

#pragma region	�ˉe�ϊ��s��
	//����p
	float fovAngle = DirectX::XMConvertToRadians(45.f);
	//�A�X�y�N�g��
	float aspectRatio = (float)16/9;
	//�[�x
	float nearZ = 0.1f;
	float farZ = 10000.f;
#pragma endregion

	//�r���[
	DirectX::XMMATRIX matView;
	//�ˉe
	DirectX::XMMATRIX matProjection;

	/// <summary>
	/// ������
	/// </summary>
	void Initialize();

	/// <summary>
	/// �萔�o�b�t�@����
	/// </summary>
	void CreateConstBuffer();

	/// <summary>
	/// �}�b�s���O
	/// </summary>
	void Map();

	/// <summary>
	/// �s��X�V
	/// </summary>
	void UpdateMatrix();
};

