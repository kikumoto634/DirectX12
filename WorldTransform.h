#pragma once
#include <DirectXMath.h>
#include <d3d12.h>
#include <wrl.h>

//�萔�o�b�t�@
struct ConstBufferDataWorldTransform
{
	DirectX::XMMATRIX matWorld;
};

/// <summary>
/// ���[���h�ϊ�
/// </summary>
struct WorldTransform
{
	//�萔�o�b�t�@
	Microsoft::WRL::ComPtr<ID3D12Resource> constBuff;
	//�}�b�s���O
	ConstBufferDataWorldTransform* constMap = nullptr;
	//�X�P�[��
	DirectX::XMFLOAT3 scale = {1,1,1};
	//��]�p
	DirectX::XMFLOAT3 rotation = {0,0,0};
	//���W
	DirectX::XMFLOAT3 position = {0,0,0};
	//���[���h
	DirectX::XMMATRIX matWorld;
	//�e
	WorldTransform* parent = nullptr;

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

