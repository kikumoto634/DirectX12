#include "TextureManager.h"
#include <assert.h>



uint32_t TextureManager::Load(const std::string& fileName)
{
	return TextureManager::GetInstance()->LoadInternal(fileName);
}

TextureManager *TextureManager::GetInstance()
{
	static TextureManager instance;
	return &instance;
}

void TextureManager::Initialize(ID3D12Device* device, std::string directoryPath)
{
	assert(device);

	this->device = device;
	this->directoryPath = directoryPath;

	//�f�X�N���v�^�T�C�Y���擾
	descriptorHandleSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//�S�e�N�X�`�����Z�b�g
	ResetAll();
}

void TextureManager::ResetAll()
{
	HRESULT result = S_FALSE;

	//�f�X�N���v�^�q�[�v����
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NumDescriptors = kNumDescriptors;
	result = device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	assert(SUCCEEDED(result));

	indexDescriptorHeap = 0;

	//�S�e�N�X�`��������
	for(size_t i = 0; i < kNumDescriptors; i++)
	{
		texture[i].resource.Reset();
		texture[i].cpuDescHandleSRV.ptr = 0;
		texture[i].gpuDescHandleSRV.ptr = 0;
		texture[i].name.clear();
	}
}

uint32_t TextureManager::LoadInternal(const std::string &fileName)
{
	assert(indexDescriptorHeap < kNumDescriptors);
	uint32_t handle = indexDescriptorHeap;

	// �ǂݍ��ݍς݃e�N�X�`��������
	auto it = std::find_if(texture.begin(), texture.end(), [&](const auto& texture) {
		return texture.name == fileName;
	});
	if (it != texture.end()) {
		// �ǂݍ��ݍς݃e�N�X�`���̗v�f�ԍ����擾
		handle = static_cast<uint32_t>(std::distance(texture.begin(), it));
		return handle;
	}

	// �������ރe�N�X�`���̎Q��
	Texture& texture = this->texture.at(handle);
	texture.name = fileName;

	// �f�B���N�g���p�X�ƃt�@�C������A�����ăt���p�X�𓾂�
	bool currentRelative = false;
	if (2 < fileName.size()) {
		currentRelative = (fileName[0] == '.') && (fileName[1] == '/');
	}
	std::string fullPath = currentRelative ? fileName : directoryPath + fileName;

	// ���j�R�[�h������ɕϊ�
	wchar_t wfilePath[256];
	MultiByteToWideChar(CP_ACP, 0, fullPath.c_str(), -1, wfilePath, _countof(wfilePath));

	HRESULT result = S_FALSE;

	TexMetadata metadata{};
	ScratchImage scratchImg{};

	// WIC�e�N�X�`���̃��[�h
	result = LoadFromWICFile(wfilePath, WIC_FLAGS_NONE, &metadata, scratchImg);
	assert(SUCCEEDED(result));

	ScratchImage mipChain{};
	// �~�b�v�}�b�v����
	result = GenerateMipMaps(
	  scratchImg.GetImages(), scratchImg.GetImageCount(), scratchImg.GetMetadata(),
	  TEX_FILTER_DEFAULT, 0, mipChain);
	if (SUCCEEDED(result)) {
		scratchImg = std::move(mipChain);
		metadata = scratchImg.GetMetadata();
	}

	// �ǂݍ��񂾃f�B�t���[�Y�e�N�X�`����SRGB�Ƃ��Ĉ���
	metadata.format = MakeSRGB(metadata.format);

	// ���\�[�X�ݒ�
	CD3DX12_RESOURCE_DESC texresDesc = CD3DX12_RESOURCE_DESC::Tex2D(
	  metadata.format, metadata.width, (UINT)metadata.height, (UINT16)metadata.arraySize,
	  (UINT16)metadata.mipLevels);

	// �q�[�v�v���p�e�B
	CD3DX12_HEAP_PROPERTIES heapProps =
	  CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0);

	// �e�N�X�`���p�o�b�t�@�̐���
	result = device->CreateCommittedResource(
	  &heapProps, D3D12_HEAP_FLAG_NONE, &texresDesc,
	  D3D12_RESOURCE_STATE_GENERIC_READ, // �e�N�X�`���p�w��
	  nullptr, IID_PPV_ARGS(&texture.resource));
	assert(SUCCEEDED(result));

	// �e�N�X�`���o�b�t�@�Ƀf�[�^�]��
	for (size_t i = 0; i < metadata.mipLevels; i++) {
		const Image* img = scratchImg.GetImage(i, 0, 0); // ���f�[�^���o
		result = texture.resource->WriteToSubresource(
		  (UINT)i,
		  nullptr,              // �S�̈�փR�s�[
		  img->pixels,          // ���f�[�^�A�h���X
		  (UINT)img->rowPitch,  // 1���C���T�C�Y
		  (UINT)img->slicePitch // 1���T�C�Y
		);
		assert(SUCCEEDED(result));
	}

	// �V�F�[�_���\�[�X�r���[�쐬
	texture.cpuDescHandleSRV = CD3DX12_CPU_DESCRIPTOR_HANDLE(
	  descriptorHeap->GetCPUDescriptorHandleForHeapStart(), handle, descriptorHandleSize);
	texture.gpuDescHandleSRV = CD3DX12_GPU_DESCRIPTOR_HANDLE(
	  descriptorHeap->GetGPUDescriptorHandleForHeapStart(), handle, descriptorHandleSize);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{}; // �ݒ�\����
	D3D12_RESOURCE_DESC resDesc = texture.resource->GetDesc();

	srvDesc.Format = resDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2D�e�N�X�`��
	srvDesc.Texture2D.MipLevels = (UINT)metadata.mipLevels;

	device->CreateShaderResourceView(
	  texture.resource.Get(), //�r���[�Ɗ֘A�t����o�b�t�@
	  &srvDesc,               //�e�N�X�`���ݒ���
	  texture.cpuDescHandleSRV);

	indexDescriptorHeap++;

	return handle;
}

const D3D12_RESOURCE_DESC TextureManager::GetResourceDesc(uint32_t textureHandle)
{
	assert(textureHandle < texture.size());
	Texture& texture = this->texture.at(textureHandle);
	return texture.resource->GetDesc();
}

void TextureManager::SetGraphicsRootDescriptorTable(ID3D12GraphicsCommandList *commandList, UINT rootParamIndex, uint32_t textureHandle)
{
	assert(textureHandle < texture.size());
	//�e�X�N�`���p�f�X�N���v�^�q�[�v�̐ݒ�
	ID3D12DescriptorHeap* ppHeaps[] = {descriptorHeap.Get()};
	commandList->SetDescriptorHeaps(_countof(ppHeaps),ppHeaps);

	//�V�F�[�_�[���\�[�X�r���[(SRV)���Z�b�g
	commandList->SetGraphicsRootDescriptorTable(rootParamIndex, texture[textureHandle].gpuDescHandleSRV);
}
