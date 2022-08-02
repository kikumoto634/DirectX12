#include "TextureManager.h"
#include <assert.h>
#include <DirectXTex.h>


using namespace DirectX;

uint32_t TextureManager::Load(const std::string &fileName)
{
	return TextureManager::GetInstance()->LoadInternal(fileName);
}

TextureManager *TextureManager::GetInstance()
{
	static TextureManager instance;
	return &instance;
}

void TextureManager::Initialize(ID3D12Device *device, std::string directoryPath)
{
	assert(device);

	this->device = device;
	this->directoryPath = directoryPath;

	//�f�X�N���v�^�T�C�Y���擾
	descriptorHandleIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//�S�e�N�X�`�����Z�b�g
	ResetAll();
}

void TextureManager::ResetAll()
{
	HRESULT result = S_FALSE;

	//�f�X�N���v�^�q�[�v����
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc= {};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NumDescriptors= kNumDescriptors;
	result = this->device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&desciptorHeap));
	assert(SUCCEEDED(result));

	indexNextDescriptorHeap = 0;

	//�S�e�N�X�`��������
	for(size_t i = 0; i < kNumDescriptors; i++)
	{
		textures[i].resource.Reset();
		textures[i].cpuDescHandleSRV.ptr = 0;
		textures[i].gpuDeschandleSRV.ptr = 0;
		textures[i].name.clear();
	}
}

const D3D12_RESOURCE_DESC TextureManager::GetResourceDesc(uint32_t texturehandle)
{
	assert(texturehandle < textures.size());
	Texture& texture = textures.at(texturehandle);
	return texture.resource->GetDesc();
}

void TextureManager::SetGraphicsRootDescriptorTable(ID3D12GraphicsCommandList *commandList, UINT rootParamIndex, uint32_t textureHandle)
{
	assert(textureHandle < textures.size());
	//�f�X�N���v�^�q�[�v�ɔz��
	ID3D12DescriptorHeap* ppHeaps[] = {desciptorHeap.Get()};
	commandList->SetDescriptorHeaps(_countof(ppHeaps),ppHeaps);

	//�V�F�[�_�[���\�[�X�r���[���Z�b�g
	commandList->SetGraphicsRootDescriptorTable(rootParamIndex, textures[textureHandle].gpuDeschandleSRV);
}

uint32_t TextureManager::LoadInternal(const std::string &fileName)
{
	assert(indexNextDescriptorHeap < kNumDescriptors);
	uint32_t handle = indexNextDescriptorHeap;

	//�ǂݍ��ݍς݃e�N�X�`��������
	auto it = std::find_if(textures.begin(), textures.end(), [&](const auto& texture)
	{
		return texture.name == fileName;
	});
	if(it !=textures.end())
	{
		//�ǂݍ��ݍς݃e�N�X�`���̗v�f�ԍ����擾
		handle = static_cast<uint32_t>(std::distance(textures.begin(), it));
		return handle;	
	}

	//�ǂݍ��݃e�N�X�`���̎Q��
	Texture& texture = textures.at(handle);
	texture.name = fileName;

	//�f�B���N�g���p�X�ƃt�@�C������A�����ăt���p�X�𓾂�
	bool currentRelative = false;
	if(2 < fileName.size())
	{
		currentRelative = (fileName[0] == '.') && (fileName[1] == '/');
	}
	std::string fullPath = currentRelative ? fileName : directoryPath + fileName;

	//���j�R�[�h������ɕϊ�
	wchar_t wfilePath[256];
	MultiByteToWideChar(CP_ACP, 0, fullPath.c_str(), -1, wfilePath, _countof(wfilePath));

	HRESULT result = S_FALSE;

	TexMetadata metadata{};
	ScratchImage scratchImg{};

	//WIC�e�N�X�`���̃��[�h
	result = LoadFromWICFile(wfilePath, WIC_FLAGS_NONE, &metadata, scratchImg);
	assert(SUCCEEDED(result));

	ScratchImage mipChain{};
	//�~�b�v�}�b�v����
	result = GenerateMipMaps(scratchImg.GetImages(), scratchImg.GetImageCount(), scratchImg.GetMetadata(), TEX_FILTER_DEFAULT, 0, mipChain);
	if(SUCCEEDED(result))
	{
		scratchImg = std::move(mipChain);
		metadata = scratchImg.GetMetadata();
	}

	//�ǂݍ��񂾃f�B�q���[�Y�e�N�X�`����SRGB�Ƃ��Ĉ���
	metadata.format = MakeSRGB(metadata.format);

	//���\�[�X�ݒ�
	CD3DX12_RESOURCE_DESC texresDesc = CD3DX12_RESOURCE_DESC::Tex2D( metadata.format, metadata.width, (UINT)metadata.height, (UINT16)metadata.arraySize, (UINT16)metadata.mipLevels);

	//�q�[�v�v���p�e�B
	CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0);

	//�e�N�X�`���p�o�b�t�@����
	for(size_t i = 0; i < metadata.mipLevels; i++)
	{
		const Image*img = scratchImg.GetImage(i, 0, 0);
		result= texture.resource->WriteToSubresource
			(
				(UINT)i,
				nullptr,
				img->pixels,
				(UINT)img->rowPitch,
				(UINT)img->slicePitch
			);
		assert(SUCCEEDED(result));
	}

	//�V�F�[�_�[���\�[�X�r���[����
	texture.cpuDescHandleSRV = CD3DX12_CPU_DESCRIPTOR_HANDLE
	(
		desciptorHeap->GetCPUDescriptorHandleForHeapStart(),
		handle,
		descriptorHandleIncrementSize
	);
	texture.gpuDeschandleSRV = CD3DX12_GPU_DESCRIPTOR_HANDLE
	(
		desciptorHeap->GetGPUDescriptorHandleForHeapStart(),
		handle,
		descriptorHandleIncrementSize
	);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	D3D12_RESOURCE_DESC resDesc = texture.resource->GetDesc();

	srvDesc.Format = resDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = (UINT)metadata.mipLevels;

	device->CreateShaderResourceView
		(
			texture.resource.Get(),
			&srvDesc,
			texture.cpuDescHandleSRV
		);

	indexNextDescriptorHeap++;

	return handle;
}
