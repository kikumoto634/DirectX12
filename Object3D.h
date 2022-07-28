#pragma once
#include "Model.h"
#include "Camera.h"

#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <DirectXMath.h>
#include <string>

class Object3D
{
/// <summary>
/// �G�C���A�X
/// </summary>
protected:
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMMATRIX = DirectX::XMMATRIX;

public:
	//�T�u�N���X
	//�萔�o�b�t�@�p�f�[�^�\����(���W�ϊ��s��p)
	struct ConstBufferDataTransform
	{
		XMMATRIX viewproj;	//�r���[�v���W�F�N�V����
		XMMATRIX world;		//���[���h�s��
		XMFLOAT3 cameraPos;	//�J�������W(���[���h���W)
	};

/// <summary>
/// �ÓI�����o�֐�
/// </summary>
public:
	//setter
	static void SetDevice(ID3D12Device* device) {Object3D::device = device; }
	static void SetCamera(Camera* camera)	{Object3D::camera = camera;}

	/// <summary>
	/// �O���t�B�b�N�X�p�C�v���C���̐���
	/// </summary>
	static void CreateGraphicsPipeline();

/// <summary>
/// �����o�֐�
/// </summary>
public:

	/// <summary>
	/// ������
	/// </summary>
	void Initialize();

	/// <summary>
	/// �X�V
	/// </summary>
	void Update();

	/// <summary>
	/// �`��
	/// </summary>
	void Draw(ID3D12GraphicsCommandList* commandList);

	/// <summary>
	/// ���f���Z�b�g
	/// </summary>
	/// <param name="model">���f��</param>
	void SetModel(Model* model)	{this->model = model;}


	//setter
	void SetScale(XMFLOAT3 scale)	{this->scale = scale;}
	void SetRotation(XMFLOAT3 rotation)	{this->rotation = rotation;}
	void SetPosition(XMFLOAT3 position)	{this->position = position;}

	//getter
	XMFLOAT3 GetScale()	{return this->scale;}
	XMFLOAT3 GetRotation()	{return this->rotation;}
	XMFLOAT3 GetPosition()	{return this->position;}


/// <summary>
/// �ÓI�����o�ϐ�
/// </summary>
private:
	static ID3D12Device* device;
	static Camera* camera;

	//���[�g�V�O�l�`��
	static ComPtr<ID3D12RootSignature> rootsignature;
	//�p�C�v���C���X�e�[�g�I�u�W�F�N�g
	static ComPtr<ID3D12PipelineState> pipelinestate;

/// <summary>
/// �����o�ϐ�
/// </summary>
private:

	//�萔�o�b�t�@
	ComPtr<ID3D12Resource> constBuffTransform;

	//���[�J���X�P�[��
	XMFLOAT3 scale = {1, 1, 1};
	//X,Y,Z������̃��[�J����]�p
	XMFLOAT3 rotation = {0, 0, 0};
	//���[�J�����W
	XMFLOAT3 position = {0, 0, 0};
	//���[�J�����[���h�ϊ��s��
	XMMATRIX matWorld{};
	//���f��
	Model* model = nullptr;
};

