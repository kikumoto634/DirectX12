#include "TextureManager.h"
#include <assert.h>

void TextureManager::Initialize(DirectXCommon* dxCommon)
{
	HRESULT result;

	//�����o�֐��ɋL�^
	this->dxCommon = dxCommon;

	//�f�X�N���v�^�q�[�v�̐���
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc= {};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NumDescriptors= maxTextureCount;
	result = this->dxCommon->GetDevice()->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&descHeap));
	if(FAILED(result))
	{
		assert(0);
	}
}

void TextureManager::LoadTexture(UINT texnumber, const wchar_t *filename)
{
	//�ُ�Ȕԍ��̎w������o
	assert(texnumber <= maxTextureCount - 1);

	HRESULT result;

	///�摜�t�@�C���̗p��
	TexMetadata metadata{};
	ScratchImage scratchImg{};
	//WIC�e�N�X�`���f�[�^�̃��[�h
	result = LoadFromWICFile(
		filename,
		WIC_FLAGS_NONE,
		&metadata, scratchImg);
	if(FAILED(result))
	{
		assert(0);
	}


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
			IID_PPV_ARGS(&textureBuffer[texnumber])
		);
	if(FAILED(result))
	{
		assert(0);
	}


	//�e�N�X�`���o�b�t�@�ւ̃f�[�^�]��
	//�S�~�b�v�}�b�v�ɂ���
	for(size_t i = 0; i < metadata.mipLevels; i++)
	{
		//�~�b�v�}�b�v���x�����w�肵�ăC���[�W���擾
		const Image* img = scratchImg.GetImage(i, 0, 0);
		//�e�N�X�`���o�b�t�@�Ƀf�[�^�]��
		result = textureBuffer[texnumber]->WriteToSubresource(
			(UINT)i,				
			nullptr,				//�S�̈�փR�s�[
			img->pixels,			//���f�[�^�A�h���X
			(UINT)img->rowPitch,	//�ꃉ�C���T�C�Y
			(UINT)img->slicePitch	//�ꖇ�T�C�Y
		);
		if(FAILED(result))
		{
			assert(0);
		}
	}

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
		textureBuffer[texnumber].Get(),
		&srvDesc,
		CD3DX12_CPU_DESCRIPTOR_HANDLE//SRV�q�[�v�̐擪�n���h�����擾
			(
				descHeap->GetCPUDescriptorHandleForHeapStart(),
				texnumber,
				dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
			)
	);
}

ID3D12Resource *TextureManager::GetSpriteTexBuffer(UINT texnumber)
{
	//�z��I�[�o�[�t���[�h�~
	assert(texnumber < maxTextureCount);

	return textureBuffer[texnumber].Get();
}

void TextureManager::SetDescriptorHeaps(ID3D12GraphicsCommandList *commandList)
{
	//�e�X�N�`���p�f�X�N���v�^�q�[�v�̐ݒ�
	ID3D12DescriptorHeap* ppHeaps[] = {descHeap.Get()};
	commandList->SetDescriptorHeaps(_countof(ppHeaps),ppHeaps);
}

void TextureManager::SetShaderResourceView(ID3D12GraphicsCommandList *commandList, UINT RootParameterIndex, UINT texnumber)
{
	//SRV�̃T�C�Y���擾
	UINT sizeSRV = dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//�f�X�N���v�^�e�[�u���̐擪���擾
	D3D12_GPU_DESCRIPTOR_HANDLE start = descHeap->GetGPUDescriptorHandleForHeapStart();

	//SRV��GPU�n���h���擾
	CD3DX12_GPU_DESCRIPTOR_HANDLE handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(start, texnumber,sizeSRV);

	//�V�F�[�_�[���\�[�X�r���[(SRV)���Z�b�g
	commandList->SetGraphicsRootDescriptorTable(RootParameterIndex, handle);
}