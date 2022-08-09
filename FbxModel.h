#pragma once
#include "FbxModelMesh.h"
#include "WorldTransform.h"
#include "ViewProjection.h"

#include <wrl.h>
#include <DirectXMath.h>
#include <string>
#include <vector>

/// <summary>
/// Fbx���f��
/// </summary>
class FbxModel
{
private:	//�G�C���A�X
	template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMMATRIX = DirectX::XMMATRIX;
	using string = std::string;

public:	//��
	enum class RootParameter
	{
		WorldTransform,
		ViewProjection,
		Texture,
	};

public:	//�萔
	static const int MAX_BONES = 32;


public:	//�T�u�N���X
	//�萔�o�b�t�@�p�f�[�^�\��(�X�L�j���O)
	struct ConstBufferDataSkin
	{
		XMMATRIX bones[MAX_BONES];
	};

private://�ÓI�����o�֐�

	static void StaticInitialize();

	static void InitializeGraphicsPipeline();

	static FbxModel* Create();

	static void PreDraw(ID3D12GraphicsCommandList* commandList);

	static void PostDraw();

public:	//�����o�֐�
	
	~FbxModel();

	void Initialize();

	void Draw(const WorldTransform& worldTransform, const ViewProjection& viewProjection);

	void SetModel();

	void PlayAnimation();

private:	//�ÓI�����o�ϐ�
	static UINT sDescriptorHandle;
	static ID3D12GraphicsCommandList* sCommandList;
	static ComPtr<ID3D12RootSignature> sRootSignature;
	static ComPtr<ID3D12PipelineState> sPipelineState;

private:	//�����o�ϐ�

	ComPtr<ID3D12Resource> constBufferSkin;

	FbxTime frameTime;
	

	std::vector<FbxModelMesh*> mesh;
};

