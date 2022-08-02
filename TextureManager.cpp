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

	//デスクリプタサイズを取得
	descriptorHandleIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//全テクスチャリセット
	ResetAll();
}

void TextureManager::ResetAll()
{
	HRESULT result = S_FALSE;

	//デスクリプタヒープ生成
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc= {};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NumDescriptors= kNumDescriptors;
	result = this->device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&desciptorHeap));
	assert(SUCCEEDED(result));

	indexNextDescriptorHeap = 0;

	//全テクスチャ初期化
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
	//デスクリプタヒープに配列
	ID3D12DescriptorHeap* ppHeaps[] = {desciptorHeap.Get()};
	commandList->SetDescriptorHeaps(_countof(ppHeaps),ppHeaps);

	//シェーダーリソースビューをセット
	commandList->SetGraphicsRootDescriptorTable(rootParamIndex, textures[textureHandle].gpuDeschandleSRV);
}

uint32_t TextureManager::LoadInternal(const std::string &fileName)
{
	assert(indexNextDescriptorHeap < kNumDescriptors);
	uint32_t handle = indexNextDescriptorHeap;

	//読み込み済みテクスチャを検索
	auto it = std::find_if(textures.begin(), textures.end(), [&](const auto& texture)
	{
		return texture.name == fileName;
	});
	if(it !=textures.end())
	{
		//読み込み済みテクスチャの要素番号を取得
		handle = static_cast<uint32_t>(std::distance(textures.begin(), it));
		return handle;	
	}

	//読み込みテクスチャの参照
	Texture& texture = textures.at(handle);
	texture.name = fileName;

	//ディレクトリパスとファイル名を連結してフルパスを得る
	bool currentRelative = false;
	if(2 < fileName.size())
	{
		currentRelative = (fileName[0] == '.') && (fileName[1] == '/');
	}
	std::string fullPath = currentRelative ? fileName : directoryPath + fileName;

	//ユニコード文字列に変換
	wchar_t wfilePath[256];
	MultiByteToWideChar(CP_ACP, 0, fullPath.c_str(), -1, wfilePath, _countof(wfilePath));

	HRESULT result = S_FALSE;

	TexMetadata metadata{};
	ScratchImage scratchImg{};

	//WICテクスチャのロード
	result = LoadFromWICFile(wfilePath, WIC_FLAGS_NONE, &metadata, scratchImg);
	assert(SUCCEEDED(result));

	ScratchImage mipChain{};
	//ミップマップ生成
	result = GenerateMipMaps(scratchImg.GetImages(), scratchImg.GetImageCount(), scratchImg.GetMetadata(), TEX_FILTER_DEFAULT, 0, mipChain);
	if(SUCCEEDED(result))
	{
		scratchImg = std::move(mipChain);
		metadata = scratchImg.GetMetadata();
	}

	//読み込んだディヒューズテクスチャをSRGBとして扱う
	metadata.format = MakeSRGB(metadata.format);

	//リソース設定
	CD3DX12_RESOURCE_DESC texresDesc = CD3DX12_RESOURCE_DESC::Tex2D( metadata.format, metadata.width, (UINT)metadata.height, (UINT16)metadata.arraySize, (UINT16)metadata.mipLevels);

	//ヒーププロパティ
	CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0);

	//テクスチャ用バッファ生成
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

	//シェーダーリソースビュー生成
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
