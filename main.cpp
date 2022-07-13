#include "WinApp.h"

//��{(������)
#include "DirectXCommon.h"
#include <cassert>
//�}�`�`��
#include <DirectXMath.h>
//D3D�R���p�C���̃C���N���[�h
#include <d3dcompiler.h>
//�L�[�{�[�h����
#include "Input.h"
//DirectXTex����
#include "DirectXTex.h"
//�e�N�X�`���}�l�[�W���[
#include "TextureManager.h"
//ComPtr�X�}�[�g�|�C���^
#include <wrl.h>
//�X�v���C�g
#include "Sprite.h"
//�T�E���h
//#include <xaudio2.h>
#include "SoundManager.h"
//�t�@�C���ǂݍ���
//#include <fstream>

//����
#pragma comment(lib, "d3dcompiler.lib")
//#pragma comment(lib, "xaudio2.lib")

using namespace DirectX;
using namespace Microsoft::WRL;

//CD3DX12�w���p�[�\����
//#include <d3dx12.h>



//���_�f�[�^�\����(3D)
struct Vertex
{
	XMFLOAT3 pos;	//xyz���W
	XMFLOAT3 normal;//�@���x�N�g��
	XMFLOAT2 uv;	//uv���W
};


//�萔�o�b�t�@�p�f�[�^�\����(3D�ϊ��s��
struct ConstBufferData{
	XMMATRIX mat;	//3D�ϊ��s��
	XMFLOAT4 color;	//�F(RGBA)
};

//3D�I�u�W�F�N�g�^
struct Object3d
{
	//�}�b�s���O�p�|�C���^
	ConstBufferData* constBuffer = nullptr;
	//�F
	XMFLOAT4 color = {1.0f, 1.0f, 1.0f, 1.0f};
	//�萔�o�b�t�@(�s��p)
	ComPtr<ID3D12Resource> constBuffData = nullptr;
	//�A�t�B���ϊ�
	XMFLOAT3 scale = {1.0f, 1.0f, 1.0f};
	XMFLOAT3 rotation = {0.0f, 0.0f, 0.0f};
	XMFLOAT3 position = {0.0f, 0.0f, 0.0f};
	//���[���h�ϊ��s��
	XMMATRIX matWorld;
	//�e�I�u�W�F�N�g�ւ̃|�C���^
	Object3d* parent = nullptr;
};

//�p�C�v���C���Z�b�g
struct PipelineSet
{
	//�p�C�v���C���X�e�[�g�I�u�W�F�N�g
	ComPtr<ID3D12PipelineState> pipelinestate;
	//���[�g�V�O�l�`��
	ComPtr<ID3D12RootSignature> rootsignature;
};

////�f�o�b�N�p������N���X�̒�`
//class DebugText
//{
//public:
//
//	static const int maxCharCount = 256;
//	static const int fontWidth = 9;
//	static const int fontHeight = 18;
//	static const int fontLineCount = 14;
//
//private:
//	Sprite sprite[maxCharCount];
//	int spriteIndex = 0;
//};


//3D�I�u�W�F�N�g�p�p�C�v���C������
PipelineSet Object3dCreateGraphicsPipeline(ID3D12Device* device);

//3D���ʃO���t�B�b�N�X�R�}���h�̃Z�b�g
void Object3DCommonBeginDraw(ID3D12GraphicsCommandList* commandList, const PipelineSet& pipelineSet, ID3D12DescriptorHeap* descHeap);

//3D�I�u�W�F�N�g������
void InitializeObject3d(Object3d* object, ID3D12Device* device);

//�X�V
void UpdateObject3d(Object3d* object, XMMATRIX& matView, XMMATRIX& matProjection);

//�`��
void DrawObject3d(Object3d* object, ID3D12GraphicsCommandList* commandList, D3D12_VERTEX_BUFFER_VIEW& vbView, D3D12_INDEX_BUFFER_VIEW& ibView, ID3D12DescriptorHeap* srvHeap, UINT numIndices);


//WindowsAPI�I�u�W�F�N�g
WinApp* winApp = nullptr;
//DirectX�I�u�W�F�N�g
DirectXCommon* dxCommon = nullptr;
//�e�N�X�`���}�l�[�W���[
TextureManager* textureManager = nullptr;
//�X�v���C�g
const int TextureNum = 2;
Sprite* sprite = nullptr;

//�T�E���h�}�l�[�W���[
SoundManager* soundManager = nullptr;

/// Windows�A�v���ł̃G���g���[�|�C���g(main�֐�)
int WINAPI WinMain(HINSTANCE, HINSTANCE,LPSTR,int)
{
	//WindowsAPI
	winApp = new WinApp();
	//WinApp������
	winApp->Initialize();

	//DirectX
	dxCommon = new DirectXCommon();
	//������
	dxCommon->Initialize(winApp);

	//�e�N�X�`���}�l�[�W���[
	textureManager = new TextureManager();
	//������
	textureManager->Initialize(dxCommon);

	/// <summary>
	/// DirectX12 ���������� ��������
	/// </summary>
	
	/// <summary>
	/// �������ϐ�
	/// </summary>
	HRESULT result;

	////3DObject�ϐ�
	///���_�o�b�t�@
	ComPtr<ID3D12Resource> vertBuff;
	///���_�o�b�t�@�r���[
	D3D12_VERTEX_BUFFER_VIEW vbView{};
	///�C���f�b�N�X�o�b�t�@
	ComPtr<ID3D12Resource> indexBuff;
	///�C���f�b�N�X�o�b�t�@�r���[
	D3D12_INDEX_BUFFER_VIEW ibView{};


	////�e�N�X�`���ϐ�
	ComPtr<ID3D12DescriptorHeap> srvHeap = nullptr;
	//�e�N�X�`���o�b�t�@�̐���
	ComPtr<ID3D12Resource> texBuff01 ;

	////�T�E���h
	soundManager = new SoundManager();
	soundManager->Initialize();

	//�ǂݍ���
	soundManager->LoadWave(0, "Resources/fanfare.wav");

	///DirectInPut
	Input* input = new Input();
	//������
	input->Initialize(winApp->GetHwnd());


	/// <summary>
	/// DirectX12 ���������� �����܂�
	/// </summary>
	

	
	/// <summary>
	/// DirectX12 �`�揉�������� ��������
	/// </summary>
	 
	///���_�f�[�^
	Vertex vertices[] = 
	{
		//�O
		{{-5.0f, -5.0f, -5.0f}, {}, {0.0f, 1.0f}},
		{{-5.0f, +5.0f, -5.0f}, {}, {0.0f, 0.0f}},
		{{+5.0f, -5.0f, -5.0f}, {}, {1.0f, 1.0f}},
		{{+5.0f, +5.0f, -5.0f}, {}, {1.0f, 0.0f}},
		//��
		{{-5.0f, -5.0f, +5.0f}, {}, {0.0f, 1.0f}},
		{{-5.0f, +5.0f, +5.0f}, {}, {0.0f, 0.0f}},
		{{+5.0f, -5.0f, +5.0f}, {}, {1.0f, 1.0f}},
		{{+5.0f, +5.0f, +5.0f}, {}, {1.0f, 0.0f}},
		//��
		{{-5.0f, -5.0f, -5.0f}, {}, {0.0f, 1.0f}},
		{{-5.0f, -5.0f, +5.0f}, {}, {0.0f, 0.0f}},
		{{-5.0f, +5.0f, -5.0f}, {}, {1.0f, 1.0f}},
		{{-5.0f, +5.0f, +5.0f}, {}, {1.0f, 0.0f}},
		//�E
		{{+5.0f, -5.0f, -5.0f}, {}, {0.0f, 1.0f}},
		{{+5.0f, -5.0f, +5.0f}, {}, {0.0f, 0.0f}},
		{{+5.0f, +5.0f, -5.0f}, {}, {1.0f, 1.0f}},
		{{+5.0f, +5.0f, +5.0f}, {}, {1.0f, 0.0f}},
		//��
		{{-5.0f, -5.0f, -5.0f}, {}, {0.0f, 1.0f}},
		{{+5.0f, -5.0f, -5.0f}, {}, {0.0f, 0.0f}},
		{{-5.0f, -5.0f, +5.0f}, {}, {1.0f, 1.0f}},
		{{+5.0f, -5.0f, +5.0f}, {}, {1.0f, 0.0f}},
		//��
		{{-5.0f, +5.0f, -5.0f}, {}, {0.0f, 1.0f}},
		{{+5.0f, +5.0f, -5.0f}, {}, {0.0f, 0.0f}},
		{{-5.0f, +5.0f, +5.0f}, {}, {1.0f, 1.0f}},
		{{+5.0f, +5.0f, +5.0f}, {}, {1.0f, 0.0f}},
	};

	///�C���f�b�N�X�f�[�^
	uint16_t indices[] = 
	{
		//�O
		0, 1, 2,
		2, 1, 3,
		//��
		5, 4, 6,
		5, 6, 7,
		//��
		8, 9, 10,
		10, 9, 11,
		//�E
		13, 12, 14,
		13, 14, 15,
		//��
		16, 17, 18,
		18, 17, 19,
		//��
		21, 20, 22,
		21, 22, 23,
	};

	///�@���v�Z
	for(int i = 0; i < _countof(indices)/3; i++)
	{//�O�p�`����ƂɌv�Z���Ă���
		//�O�p�`�ɃC���f�b�N�X�����o���āA�ꎞ�I�ȕϐ�������
		uint16_t index0 = indices[i*3+0];
		uint16_t index1 = indices[i*3+1];
		uint16_t index2 = indices[i*3+2];
		//�O�p�`���\�����钸�_���W���x�N�g���ɑ��
		XMVECTOR p0 = XMLoadFloat3(&vertices[index0].pos);
		XMVECTOR p1 = XMLoadFloat3(&vertices[index1].pos);
		XMVECTOR p2 = XMLoadFloat3(&vertices[index2].pos);
		//p0->p1�x�N�g���Ap0->p2�x�N�g���̌v�Z	(�x�N�g�����Z)
		XMVECTOR v1 = XMVectorSubtract(p1,p0);
		XMVECTOR v2 = XMVectorSubtract(p2,p0);
		//�O�ς͗������琂���ȃx�N�g��
		XMVECTOR normal = XMVector3Cross(v1,v2);
		//���K��(������1�ɂ���)
		normal = XMVector3Normalize(normal);
		//���߂��@���𒸓_�f�[�^�ɑ��
		XMStoreFloat3(&vertices[index0].normal,normal);
		XMStoreFloat3(&vertices[index1].normal,normal);
		XMStoreFloat3(&vertices[index2].normal,normal);
	}


	//���_�f�[�^�S�̂̃T�C�Y = ���_�f�[�^����̃T�C�Y * ���_�f�[�^�̗v�f��
	UINT sizeVB = static_cast<UINT>(sizeof(vertices[0]) * _countof(vertices));

	///���_�o�b�t�@�̊m��
	//����
	result = dxCommon->GetDevice()->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(sizeVB),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&vertBuff)
		);
	assert(SUCCEEDED(result));

	///���_�o�b�t�@�ւ̃f�[�^�]��
	//GPU��̃o�b�t�@�ɑΉ��������z������(���C����������)���擾
	Vertex* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	assert(SUCCEEDED(result));
	//�S���_�ɑ΂���
	for(int i = 0; i < _countof(vertices); i++)
	{
		vertMap[i] = vertices[i];	//���W�R�s�[
	}
	//�q�������
	vertBuff->Unmap(0, nullptr);


	///���_�o�b�t�@�r���[�̍쐬(GPU�ɒ��_�o�b�t�@��������I�u�W�F�N�g)
	//�쐬
	//GPU���z�A�h���X
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	//���_�o�b�t�@�̃T�C�Y
	vbView.SizeInBytes = sizeVB;
	//���_����̃f�[�^�T�C�Y
	vbView.StrideInBytes = sizeof(vertices[0]);


	///���_�C���f�b�N�X
	//�C���f�b�N�X�f�[�^�S�̂̃T�C�Y
	UINT sizeIB = static_cast<UINT>(sizeof(uint16_t) * _countof(indices));

	///�C���f�b�N�X�o�b�t�@�̐���
	//����
	result = dxCommon->GetDevice()->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(sizeIB),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&indexBuff)
		);
	assert(SUCCEEDED(result));

	///�C���f�b�N�X�o�b�t�@�ւ̃f�[�^�]��
	//�}�b�s���O
	uint16_t* indexMap = nullptr;
	result = indexBuff->Map(0, nullptr, (void**)&indexMap);
	assert(SUCCEEDED(result));
	//�S�C���f�b�N�X�ɑ΂���
	for(int i = 0; i < _countof(indices); i++)
	{
		indexMap[i] = indices[i];
	}
	//�}�b�s���O����
	indexBuff->Unmap(0, nullptr);


	///�C���f�b�N�X�o�b�t�@�r���[�̍쐬(GPU�ɃC���f�b�N�X�o�b�t�@��������I�u�W�F�N�g)
	ibView.BufferLocation = indexBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeIB;



	////�X�v���C�g���ʃf�[�^����
	Sprite::StaticInitialize(dxCommon, textureManager);

	//�X�v���C�g���ʃe�N�X�`���Ǎ�
	textureManager->LoadTexture(0, L"Resources/Texture.jpg");
	textureManager->LoadTexture(1, L"Resources/Texture2.jpg");

	//�X�v���C�g����
	sprite = new Sprite();
	sprite->Initialize(0);
	sprite->SetPosition({100, 100});
	sprite->SetSize({100,50});
	sprite->SetAnchorpoint({0.5f,0.5f});

	//3D�I�u�W�F�N�g�p�p�C�v���C������
	PipelineSet object3dPipelineSet = Object3dCreateGraphicsPipeline(dxCommon->GetDevice());



	
	///�摜�t�@�C���̗p��
	TexMetadata metadata{};
	ScratchImage scratchImg{};
	//WIC�e�N�X�`���f�[�^�̃��[�h
	result = LoadFromWICFile(
		L"Resources/Texture.jpg",
		WIC_FLAGS_NONE,
		&metadata, scratchImg);
	assert(SUCCEEDED(result));


	//�~�b�v�}�b�v�̐���
	ScratchImage mipChain{};
	//����
	result = GenerateMipMaps(
		scratchImg.GetImages(), 
		scratchImg.GetImageCount(), 
		scratchImg.GetMetadata(),
		TEX_FILTER_DEFAULT, 
		0, 
		mipChain
	);
	if(SUCCEEDED(result))
	{
		scratchImg = std::move(mipChain);
		metadata = scratchImg.GetMetadata();
	}

	//�t�H�[�}�b�g������������
	//�ǂݍ��񂾃f�B�t���[�Y�e�N�X�`����SRGB�Ƃ��Ĉ���
	metadata.format = MakeSRGB(metadata.format);


	///�e�N�X�`���o�b�t�@�ݒ�
	//�q�[�v�ݒ�
	D3D12_HEAP_PROPERTIES textureHandleProp{};
	textureHandleProp.Type = D3D12_HEAP_TYPE_CUSTOM;
	textureHandleProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	textureHandleProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	//���\�[�X�ݒ�
	CD3DX12_RESOURCE_DESC textureResourceDesc01 = CD3DX12_RESOURCE_DESC::Tex2D
		(
			metadata.format,
			metadata.width,
			(UINT)metadata.height,
			(UINT16)metadata.arraySize,
			(UINT16)metadata.mipLevels
		);

	//�e�N�X�`���o�b�t�@�̐���
	result= dxCommon->GetDevice()->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0),
			D3D12_HEAP_FLAG_NONE,
			&textureResourceDesc01,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&texBuff01)
		);
	assert(SUCCEEDED(result));


	//�e�N�X�`���o�b�t�@�ւ̃f�[�^�]��
	//�S�~�b�v�}�b�v�ɂ���
	for(size_t i = 0; i < metadata.mipLevels; i++)
	{
		//�~�b�v�}�b�v���x�����w�肵�ăC���[�W���擾
		const Image* img = scratchImg.GetImage(i, 0, 0);
		//�e�N�X�`���o�b�t�@�Ƀf�[�^�]��
		result = texBuff01->WriteToSubresource(
			(UINT)i,				
			nullptr,				//�S�̈�փR�s�[
			img->pixels,			//���f�[�^�A�h���X
			(UINT)img->rowPitch,	//�ꃉ�C���T�C�Y
			(UINT)img->slicePitch	//�ꖇ�T�C�Y
		);
		assert(SUCCEEDED(result));
	}


	///�f�X�N���v�^�q�[�v����
	//SRV�̍ő��
	const size_t kMaxSRVCount = 2056;
	//�f�X�N���v�^�q�[�v�̐ݒ�
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;	//�V�F�[�_�[���猩����悤��
	srvHeapDesc.NumDescriptors = kMaxSRVCount;
	//�ݒ�����Ƃ�SRV�p�f�X�N���v�^�q�[�v�𐶐�
	result = dxCommon->GetDevice()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvHeap));
	assert(SUCCEEDED(result));

	///�V�F�[�_���\�[�X�r���[�̍쐬
	//�ݒ�
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};	//�ݒ�\����
	srvDesc.Format = textureResourceDesc01.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;	//2D�e�N�X�`��
	srvDesc.Texture2D.MipLevels = textureResourceDesc01.MipLevels;

	//�n���h���̎w���ʒu�ɃV�F�[�_�[���\�[�X�r���[�̍쐬
	dxCommon->GetDevice()->CreateShaderResourceView
	(
		texBuff01.Get(),
		&srvDesc,
		CD3DX12_CPU_DESCRIPTOR_HANDLE//SRV�q�[�v�̐擪�n���h�����擾
			(
				srvHeap->GetCPUDescriptorHandleForHeapStart(),
				0,
				dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
			)
	);


	//�萔�o�b�t�@ Mat	
	//�������e
	XMMATRIX matProjection;
	//�r���[�ϊ��s��
	XMMATRIX matView;
	XMFLOAT3 eye = {0.0f, 0.0f, -100.0f};	//���_���W
	XMFLOAT3 target= {0, 0, 0};//�����_���W
	XMFLOAT3 up = {0, 1, 0};	//������x�N�g��

	//3D�I�u�W�F�N�g��
	const size_t kObjectCount = 2;
	//3d�I�u�W�F�N�g�̔z��
	Object3d object3ds[kObjectCount];
		
	//�z����̑S�I�u�W�F�N�g�ɑ΂���
	for(size_t i = 0; i < _countof(object3ds); i++)
	{
		//������
		object3ds[0].color = {0.8f, 0.8f, 0.8f, 1.0f};
		object3ds[1].color = {0.2f, 0.2f, 0.2f, 1.0f};
		InitializeObject3d(&object3ds[i], dxCommon->GetDevice());

		//�e�q�\����
		//�擪�ȊO
		if(i > 0)
		{
			//��O�̃I�u�W�F�N�g��e�Ƃ���
			object3ds[i].parent = &object3ds[i - 1];

			//Scale
			object3ds[i].scale = {1.f, 1.f, 1.f};
			//rotation
			object3ds[i].rotation = {0.0f, 0.0f, XMConvertToRadians(45.0f)};
			//position
			object3ds[i].position = {0.0f, 0.0f, 4.0f};
		}
	}

	

	//�������e
	matProjection = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(45.0f),	//�㉺��p45��
		(float)WinApp::window_width / WinApp::window_height,			//aspect��(��ʉ���/��ʏc��)
		0.1f, 1000.0f				//�O�[�A���[
	);

	//�r���[�ϊ��s��
	matView = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));	


	/// <summary>
	/// DirectX12 �`�揉�������� �����܂�
	/// </summary>
	 

	float angle = 0.0f;//�J�����̉�]�p


	/// <summary>
	/// �Q�[�����[�v
	/// </summary>
	while(true)
	{
		//WindowsAPI���t���[������
		bool msgExit = winApp->Update();
		if(msgExit)
		{
			break;
		}


		/// <summary>
		/// DirectX12 ���t���[������ ��������
		/// </summary>

		///�L�[�{�[�h���̎擾�J�n
		input->Update();

		if(input->Push(DIK_1) || input->Push(DIK_2)){
			XMFLOAT2 pos = sprite->GetPosition();
			
			if(input->Push(DIK_1)){
				pos.x -= 2.0f;
			}
			if(input->Push(DIK_2)){
				pos.x += 2.0f;
			}
			
			sprite->SetPosition(pos);
		}


		//����check
		if(input->Push(DIK_D) || input->Push(DIK_A))
		{
			if(input->Push(DIK_D))angle += XMConvertToRadians(1.0f);
			else if(input->Push(DIK_A))angle -= XMConvertToRadians(1.0f);

			//angle���W�A������y���܂��ɉ�]�A���a��-100
			eye.x = -100 * sinf(angle);
			eye.z = -100 * cosf(angle);
			matView = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
		}



		if(input->Push(DIK_UP) || input->Push(DIK_DOWN) || input->Push(DIK_LEFT) || input->Push(DIK_RIGHT))
		{
			if(input->Push(DIK_UP))
			{
				object3ds[0].position.y += 1.0f;
			}
			else if(input->Push(DIK_DOWN))
			{
				object3ds[0].position.y -= 1.0f;
			}

			if(input->Push(DIK_LEFT))
			{
				object3ds[0].position.x -= 1.0f;
			}
			else if(input->Push(DIK_RIGHT))
			{
				object3ds[0].position.x += 1.0f;
			}
		}

		//�X�V����
		for(size_t i = 0; i < _countof(object3ds); i++)
		{
			UpdateObject3d(&object3ds[i], matView, matProjection);
		}

		if(input->Push(DIK_SPACE))
		{
			//�Đ�
			soundManager->PlayWave(0);
		}


		//DirectXCommon�O����
		dxCommon->BeginDraw();


		//3Dobject
		Object3DCommonBeginDraw(dxCommon->GetCommandList(), object3dPipelineSet, srvHeap.Get());

		//�S�I�u�W�F�N�g�ɂ��ď���
		for(size_t i = 0; i < _countof(object3ds); i++)
		{
			DrawObject3d(&object3ds[i], dxCommon->GetCommandList(), vbView, ibView, srvHeap.Get(), _countof(indices));
		}


		//�X�v���C�g
		Sprite::SetPipelineState(dxCommon->GetCommandList());
		sprite->Draw(dxCommon->GetCommandList());

		//DirectXCommon�`��㏈��
		dxCommon->EndDraw();
		
		/// <summary>
		/// DirectX12 ���t���[������ �����܂�
		/// </summary>
	}

	//XAudio2�̉��
	//xAudio2.Reset();
	//�����f�[�^�̊J��
	//SoundUnload(&soundData1);
	Sprite::StaticFinalize();
	delete input;
	delete soundManager;
	delete sprite;
	delete textureManager;
	delete dxCommon;
	//�Q�[���E�B���h�E�j��
	delete winApp;

	return 0;
}



//3D�I�u�W�F�N�g�p�p�C�v���C���Z�b�g
PipelineSet Object3dCreateGraphicsPipeline(ID3D12Device* device)
{
	HRESULT result;

	///���_�V�F�[�_�[file�̓ǂݍ��݂ƃR���p�C��
	ComPtr<ID3DBlob> vsBlob ;			//���_�V�F�[�_�[�I�u�W�F�N�g
	ComPtr<ID3DBlob> psBlob ;			//�s�N�Z���V�F�[�_�[�I�u�W�F�N�g
	ComPtr<ID3DBlob> errorBlob ;		//�G���[�I�u�W�F�N�g

	//���_�V�F�[�_�[�̓ǂݍ��݃R���p�C��
	result = D3DCompileFromFile(
		L"BasicVS.hlsl",		//�V�F�[�_�[�t�@�C����
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,	//�C���N���[�h�\�ɂ���
		"main", "vs_5_0",					//�G���g���[�|�C���g���A�V�F�[�_�[���f���w��
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,	//�f�o�b�N�p�ݒ�
		0,
		&vsBlob, &errorBlob);
	//�G���[�Ȃ�
	if(FAILED(result)){
		//errorBlob����G���[���e��string�^�ɃR�s�[
		std::string error;
		error.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
					errorBlob->GetBufferSize(),
					error.begin());
		error += "\n";
		//�G���[���e���o�̓E�B���h�E�ɕ\��
		OutputDebugStringA(error.c_str());
		assert(0);
	}

	//�s�N�Z���V�F�[�_�[�̓ǂݍ��݃R���p�C��
	result = D3DCompileFromFile(
		L"BasicPS.hlsl",		//�V�F�[�_�[�t�@�C����
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,	//�C���N���[�h�\�ɂ���
		"main", "ps_5_0",					//�G���g���[�|�C���g���A�V�F�[�_�[���f���w��
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,	//�f�o�b�N�p�ݒ�
		0,
		&psBlob, &errorBlob);
	//�G���[�Ȃ�
	if(FAILED(result)){
		//errorBlob����G���[���e��string�^�ɃR�s�[
		std::string error;
		error.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
					errorBlob->GetBufferSize(),
					error.begin());
		error += "\n";
		//�G���[���e���o�̓E�B���h�E�ɕ\��
		OutputDebugStringA(error.c_str());
		assert(0);
	}


	///���_���C�A�E�g
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
	
		{//xyz���W
			"POSITION",										//�Z�}���e�B�b�N��
			0,												//�����Z�}���e�B�b�N������������Ƃ��Ɏg���C���f�b�N�X
			DXGI_FORMAT_R32G32B32_FLOAT,					//�v�f���ƃr�b�g����\�� (XYZ��3��float�^�Ȃ̂�R32G32B32_FLOAT)
			0,												//���̓X���b�g�C���f�b�N�X
			D3D12_APPEND_ALIGNED_ELEMENT,					//�f�[�^�̃I�t�Z�b�g�l (D3D12_APPEND_ALIGNED_ELEMENT���Ǝ����ݒ�)
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,		//���̓f�[�^��� (�W����D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA)
			0												//��x�ɕ`�悷��C���X�^���X��
		},
		{//�@���x�N�g��
			"NORMAL",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
		{//uv���W
			"TEXCOORD",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
	};

	///���[�g�p�����[�^
	//�f�X�N���v�^�����W�̐ݒ�
	CD3DX12_DESCRIPTOR_RANGE descRangeSRV;
	descRangeSRV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);


	//�ݒ�
	////�萔�o�b�t�@ 0��
	CD3DX12_ROOT_PARAMETER rootParam[2] = {};
	////�萔�@0�� material
	rootParam[0].InitAsConstantBufferView(0);
	////�e�N�X�`�����W�X�^ 0��
	rootParam[1].InitAsDescriptorTable(1, &descRangeSRV);


	///<summmary>
	///�O���t�B�b�N�X�p�C�v���C��
	///<summary/>
	
	//�O���t�B�b�N�X�p�C�v���C���ݒ�
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc{};
	//�V�F�[�_�[�ݒ�
	pipelineDesc.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
	pipelineDesc.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());
	
	//�T���v���}�X�N�ݒ�
	pipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;	//�W���ݒ�
	//���X�^���C�U�ݒ� �w�ʃJ�����O	�|���S�����h��Ԃ�	�[�x�N���b�s���O�L��
	pipelineDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//�u�����h�X�e�[�g
	//�����_�[�^�[�Q�b�g�̃u�����h�ݒ�
	D3D12_RENDER_TARGET_BLEND_DESC& blenddesc = pipelineDesc.BlendState.RenderTarget[0];
	blenddesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;	//RGBA���ׂẴ`�����l����`��
	//���ʐݒ�
	blenddesc.BlendEnable = true;						//�u�����h��L���ɂ���
	blenddesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;		//���Z
	blenddesc.SrcBlendAlpha = D3D12_BLEND_ONE;			//�\�[�X�̒l��100% �g��	(�\�[�X�J���[			 �F ������`�悵�悤�Ƃ��Ă���F)
	blenddesc.DestBlendAlpha = D3D12_BLEND_ZERO;		//�f�X�g�̒l��  0% �g��	(�f�X�e�B�l�[�V�����J���[�F ���ɃL�����o�X�ɕ`����Ă���F)
	//�e��ݒ�
	blenddesc.BlendOp = D3D12_BLEND_OP_ADD;	//�ݒ�
	blenddesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;			//�\�[�X�̒l�� ��% �g��
	blenddesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;	//�f�X�g�̒l�� ��% �g��
	//���_���C�A�E�g�ݒ�
	pipelineDesc.InputLayout.pInputElementDescs = inputLayout;
	pipelineDesc.InputLayout.NumElements = _countof(inputLayout);
	//�}�`�̌`��ݒ� (�v���~�e�B�u�g�|���W�[)
	pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//���̑��ݒ�
	pipelineDesc.NumRenderTargets = 1;		//�`��Ώۂ͈��
	pipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;	//0~255�w���RGBA
	pipelineDesc.SampleDesc.Count = 1;	//1�s�N�Z���ɂ�1��T���v�����O
	//�f�v�X�X�e���V���X�e�[�g�̐ݒ�	(�[�x�e�X�g���s���A�������݋��A�[�x������������΋���)
	pipelineDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	pipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;	//�[�x�l�t�H�[�}�b�g

	///�e�N�X�`���T���v���[
	//�ݒ�
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0);


	//�p�C�v���C���ƃ��[�g�V�O�l�`���̃Z�b�g
	PipelineSet pipelineSet;


	//���[�g�V�O�l�`�� (�e�N�X�`���A�萔�o�b�t�@�ȂǃV�F�[�_�[�ɓn�����\�[�X�����܂Ƃ߂��I�u�W�F�N�g)
	//�ݒ�
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_0(_countof(rootParam), rootParam,1, &samplerDesc,D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	//�V���A���C�Y
	ComPtr<ID3DBlob> rootSigBlob;
	result = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob,&errorBlob);
	assert(SUCCEEDED(result));
	result = dxCommon->GetDevice()->CreateRootSignature(0,rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(),IID_PPV_ARGS(&pipelineSet.rootsignature));
	assert(SUCCEEDED(result));
	//�p�C�v���C���Ƀ��[�g�V�O�l�`�����Z�b�g
	pipelineDesc.pRootSignature = pipelineSet.rootsignature.Get();

	//�p�C�v���C���X�e�[�g (�O���t�B�b�N�X�p�C�v���C���̐ݒ���܂Ƃ߂��̂��p�C�v���C���X�e�[�g�I�u�W�F�N�g(PSO))
	//�p�C�v���C���X�e�[�g�̐���
	result = dxCommon->GetDevice()->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&pipelineSet.pipelinestate));
	assert(SUCCEEDED(result));

	//�p�C�v���C���ƃ��[�g�V�O�l�`����Ԃ�
	return pipelineSet;
}


//3DObject���ʃO���t�B�b�N�X�R�}���h�̃Z�b�g
void Object3DCommonBeginDraw(ID3D12GraphicsCommandList* commandList, const PipelineSet& pipelineSet, ID3D12DescriptorHeap* descHeap)
{
	//�p�C�v���C���X�e�[�g�̐ݒ�
	commandList->SetPipelineState(pipelineSet.pipelinestate.Get());
	//���[�g�V�O�l�`���̐ݒ�
	commandList->SetGraphicsRootSignature(pipelineSet.rootsignature.Get());
	//�v���~�e�B�u�`���ݒ�
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//�e�N�X�`���p�f�X�N���v�^�q�[�v�̐ݒ�
	ID3D12DescriptorHeap* ppHeaps[] = {descHeap};
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
}



//������
void InitializeObject3d(Object3d *object, ID3D12Device* device)
{
	HRESULT result;
	//�萔�o�b�t�@����
	result = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferData) + 0xff) & ~0xff),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&object->constBuffData)
		);
	assert(SUCCEEDED(result));

	//�萔�o�b�t�@�̃}�b�s���O
	result = object->constBuffData->Map(0,nullptr, (void**)&object->constBuffer);
	assert(SUCCEEDED(result));

	////�l���������ނƎ����I�ɓ]�������(�F�̏�����)
	//object->constBuffer->color = object->color;
}

//�X�V
void UpdateObject3d(Object3d *object, XMMATRIX &matView, XMMATRIX &matProjection)
{
	XMMATRIX matScale, matRot, matTrans;

	//�X�P�[���A��]�A���s�ړ��s��̌v�Z
	matScale = XMMatrixScaling(object->scale.x, object->scale.y, object->scale.z);
	matRot = XMMatrixIdentity();
	matRot *= XMMatrixRotationZ(object->rotation.z);
	matRot *= XMMatrixRotationX(object->rotation.x);
	matRot *= XMMatrixRotationY(object->rotation.y);
	matTrans = XMMatrixTranslation(object->position.x, object->position.y, object->position.z);

	//���[���h�s��̍���
	object->matWorld = XMMatrixIdentity();	//�ό`�����Z�b�g
	object->matWorld *= matScale;			//���[���h�s��ɃX�P�[�����O�𔽉f
	object->matWorld *= matRot;				//���[���h�s��ɉ�]�𔽉f
	object->matWorld *= matTrans;			//���[���h�s��ɕ��s�ړ��𔽉f

	//�e�I�u�W�F�N�g�̑���
	if(object->parent != nullptr)
	{
		//�e�I�u�W�F�N�g�̃��[���h�s����|����
		object->matWorld *= object->parent->matWorld;
	}


	//�萔�o�b�t�@�ւ̃f�[�^�]��
	//�l���������ނƎ����I�ɓ]�������
	object->constBuffer->color = object->color;
	object->constBuffer->mat = object->matWorld * matView *matProjection;
}

void DrawObject3d(Object3d *object, ID3D12GraphicsCommandList* commandList, D3D12_VERTEX_BUFFER_VIEW &vbView, D3D12_INDEX_BUFFER_VIEW &ibView, ID3D12DescriptorHeap* srvHeap,UINT numIndices)
{

	//���_�o�b�t�@�̐ݒ�
	commandList->IASetVertexBuffers(0, 1, &vbView);
	//�C���f�b�N�X�o�b�t�@�̐ݒ�
	commandList->IASetIndexBuffer(&ibView);
	//�萔�o�b�t�@�r���[(CBV�̐ݒ�R�}���h)
	commandList->SetGraphicsRootConstantBufferView(0, object->constBuffData->GetGPUVirtualAddress());
	//�V�F�[�_���\�[�X�r���[���Z�b�g
	dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable
		(
			1, 
			CD3DX12_GPU_DESCRIPTOR_HANDLE//SRV�q�[�v�̐擪�n���h�����擾
			(
				srvHeap->GetGPUDescriptorHandleForHeapStart(),
				0,
				dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
			)
		);

	//�`��R�}���h
	commandList->DrawIndexedInstanced(numIndices,1, 0, 0, 0);
}
