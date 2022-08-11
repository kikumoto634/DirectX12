#pragma once
#include "Model.h"
#include "Camera.h"

#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <DirectXMath.h>
#include <string>

#include "FbxLoader.h"
#include "Vector2.h"
#include "Vector3.h"

class Object3D
{
/// <summary>
/// �萔
/// </summary>
public:
	//�{�[���̍ő吔
	static const int MAX_BONES = 32;

/// <summary>
/// �G�C���A�X
/// </summary>
protected:
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	using XMMATRIX = DirectX::XMMATRIX;

public:
	//�T�u�N���X
	//�萔�o�b�t�@�p�f�[�^�\����(���W�ϊ��s��p)
	struct ConstBufferDataTransform
	{
		XMMATRIX viewproj;	//�r���[�v���W�F�N�V����
		XMMATRIX world;		//���[���h�s��
		Vector3 cameraPos;	//�J�������W(���[���h���W)
	};

	//�萔�o�b�t�@�p�f�[�^�\����(�X�L�j���O)
	struct ConstBufferDataSkin
	{
		XMMATRIX bones[MAX_BONES];
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

	/// <summary>
	/// �A�j���[�V�����J�n
	/// </summary>
	void PlayAnimation();


	//setter
	void SetScale(Vector3 scale)	{this->scale = scale;}
	void SetRotation(Vector3 rotation)	{this->rotation = rotation;}
	void SetPosition(Vector3 position)	{this->position = position;}

	//getter
	Vector3 GetScale()	{return this->scale;}
	Vector3 GetRotation()	{return this->rotation;}
	Vector3 GetPosition()	{return this->position;}


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
	//�萔�o�b�t�@(�X�L��)
	ComPtr<ID3D12Resource> constBufferSkin;

	//���[�J���X�P�[��
	Vector3 scale = {1, 1, 1};
	//X,Y,Z������̃��[�J����]�p
	Vector3 rotation = {0, 0, 0};
	//���[�J�����W
	Vector3 position = {0, 0, 0};
	//���[�J�����[���h�ϊ��s��
	XMMATRIX matWorld{};
	//���f��
	Model* model = nullptr;

	//1frame�̎���
	FbxTime frameTime;
	//�A�j���[�V�����J�n����
	FbxTime startTime;
	//�A�j���[�V�����I������
	FbxTime endTime;
	//���ݎ���(�A�j���[�V����)
	FbxTime currentTime;
	//�A�j���[�V�����Đ���
	bool isPlay = false;
};

