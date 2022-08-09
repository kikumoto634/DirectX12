#pragma once
#include <DirectXMath.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <wrl.h>
#include <string>
#include <vector>
#include <fbxsdk.h>
#include <DirectXTex.h>

struct NODE
{
	//���O
	std::string name;
	//���[�J���X�P�[��
	DirectX::XMVECTOR scaling = {1, 1, 1, 0};
	//���[�J����]�p
	DirectX::XMVECTOR rotation = {0,0, 0, 0};
	//���[�J���ړ�
	DirectX::XMVECTOR translation = {0, 0, 0, 1};
	//���[�J���ό`�s��
	DirectX::XMMATRIX transform;
	//�O���[�o���ό`�s��
	DirectX::XMMATRIX globalTransform;
	//�e�m�[�h
	NODE* parent = nullptr;
};


//�{�[���\����
struct BONE
{
	//���O
	std::string name;
	//�����p���̋t�s��
	DirectX::XMMATRIX invInitialPose;
	//�N���X�^�[(FBX���̃{�[�����)
	FbxCluster* fbxCluster;
	//�R���X�g���N�^
	BONE(const std::string& name)
	{
		this->name = name;
	}
};

/// <summary>
/// Fbx���f��
/// </summary>
class FbxModelMesh
{
public: //�萔
	static const int MAX_BONE_INDICES = 4;

private:	//�G�C���A�X
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMMATRIX = DirectX::XMMATRIX;

public: //�T�u�N���X
	friend class FbxLoder;

	//���_���
	struct VertexPosNormalUVSkin
	{
		XMFLOAT3 pos;
		XMFLOAT3 normal;
		XMFLOAT3 uv;
		UINT boneIndex[MAX_BONE_INDICES];
		float boneWeight[MAX_BONE_INDICES];
	};

public: //�����o�֐�

	void CreateBuffers();

	void Draw(ID3D12GraphicsCommandList* commandList, UINT rootParameterIndex);

private: //�����o�ϐ�

	FbxScene* fbxScene = nullptr;

	std::string name;

	std::vector<NODE> nodes;

	//���b�V�������m�[�h
	NODE* meshNode= nullptr;


	//���_�o�b�t�@
	ComPtr<ID3D12Resource> vertBuff;
	//�C���f�b�N�X�o�b�t�@
	ComPtr<ID3D12Resource> indexBuff;
	//�e�N�X�`���o�b�t�@
	ComPtr<ID3D12Resource> texBuff;
	//���_�o�b�t�@�r���[
	D3D12_VERTEX_BUFFER_VIEW vbView{};
	//�C���f�b�N�X�o�b�t�@�r���[
	D3D12_INDEX_BUFFER_VIEW ibView{};
	//SRV�p�f�X�N���v�^�q�[�v
	ComPtr<ID3D12DescriptorHeap> descHeapsSRV;

	//���_�f�[�^�z��
	std::vector<VertexPosNormalUVSkin> vertices;
	//�C���f�b�N�X
	std::vector<unsigned short> indices;

	///�}�e���A��
	XMFLOAT3 ambient = {1,1,1};
	XMFLOAT3 diffuse = {1,1,1};
	DirectX::TexMetadata metaData = {};
	DirectX::ScratchImage scratchImg = {};

	//�{�[��
	std::vector<BONE> bones;
};

