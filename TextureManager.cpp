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

	//デスクリプタサイズを取得
	descriptorHandleSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//全テクスチャリセット
	ResetAll();
}

void TextureManager::ResetAll()
{
	HRESULT result = S_FALSE;

	//デスクリプタヒープ生成
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NumDescriptors = kNumDescriptors;
	result = device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	assert(SUCCEEDED(result));

	indexDescriptorHeap = 0;

	//全テクスチャ初期化
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

	// 読み込み済みテクスチャを検索
	auto it = std::find_if(texture.begin(), texture.end(), [&](const auto& texture) {
		return texture.name == fileName;
	});
	if (it != texture.end()) {
		// 読み込み済みテクスチャの要素番号を取得
		handle = static_cast<uint32_t>(std::distance(texture.begin(), it));
		return handle;
	}

	// 書き込むテクスチャの参照
	Texture& texture = this->texture.at(handle);
	texture.name = fileName;

	// ディレクトリパスとファイル名を連結してフルパスを得る
	bool currentRelative = false;
	if (2 < fileName.size()) {
		currentRelative = (fileName[0] == '.') && (fileName[1] == '/');
	}
	std::string fullPath = currentRelative ? fileName : directoryPath + fileName;

	// ユニコード文字列に変換
	wchar_t wfilePath[256];
	MultiByteToWideChar(CP_ACP, 0, fullPath.c_str(), -1, wfilePath, _countof(wfilePath));

	HRESULT result = S_FALSE;

	TexMetadata metadata{};
	ScratchImage scratchImg{};

	// WICテクスチャのロード
	result = LoadFromWICFile(wfilePath, WIC_FLAGS_NONE, &metadata, scratchImg);
	assert(SUCCEEDED(result));

	ScratchImage mipChain{};
	// ミップマップ生成
	result = GenerateMipMaps(
	  scratchImg.GetImages(), scratchImg.GetImageCount(), scratchImg.GetMetadata(),
	  TEX_FILTER_DEFAULT, 0, mipChain);
	if (SUCCEEDED(result)) {
		scratchImg = std::move(mipChain);
		metadata = scratchImg.GetMetadata();
	}

	// 読み込んだディフューズテクスチャをSRGBとして扱う
	metadata.format = MakeSRGB(metadata.format);

	// リソース設定
	CD3DX12_RESOURCE_DESC texresDesc = CD3DX12_RESOURCE_DESC::Tex2D(
	  metadata.format, metadata.width, (UINT)metadata.height, (UINT16)metadata.arraySize,
	  (UINT16)metadata.mipLevels);

	// ヒーププロパティ
	CD3DX12_HEAP_PROPERTIES heapProps =
	  CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0);

	// テクスチャ用バッファの生成
	result = device->CreateCommittedResource(
	  &heapProps, D3D12_HEAP_FLAG_NONE, &texresDesc,
	  D3D12_RESOURCE_STATE_GENERIC_READ, // テクスチャ用指定
	  nullptr, IID_PPV_ARGS(&texture.resource));
	assert(SUCCEEDED(result));

	// テクスチャバッファにデータ転送
	for (size_t i = 0; i < metadata.mipLevels; i++) {
		const Image* img = scratchImg.GetImage(i, 0, 0); // 生データ抽出
		result = texture.resource->WriteToSubresource(
		  (UINT)i,
		  nullptr,              // 全領域へコピー
		  img->pixels,          // 元データアドレス
		  (UINT)img->rowPitch,  // 1ラインサイズ
		  (UINT)img->slicePitch // 1枚サイズ
		);
		assert(SUCCEEDED(result));
	}

	// シェーダリソースビュー作成
	texture.cpuDescHandleSRV = CD3DX12_CPU_DESCRIPTOR_HANDLE(
	  descriptorHeap->GetCPUDescriptorHandleForHeapStart(), handle, descriptorHandleSize);
	texture.gpuDescHandleSRV = CD3DX12_GPU_DESCRIPTOR_HANDLE(
	  descriptorHeap->GetGPUDescriptorHandleForHeapStart(), handle, descriptorHandleSize);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{}; // 設定構造体
	D3D12_RESOURCE_DESC resDesc = texture.resource->GetDesc();

	srvDesc.Format = resDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	srvDesc.Texture2D.MipLevels = (UINT)metadata.mipLevels;

	device->CreateShaderResourceView(
	  texture.resource.Get(), //ビューと関連付けるバッファ
	  &srvDesc,               //テクスチャ設定情報
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
	//テスクチャ用デスクリプタヒープの設定
	ID3D12DescriptorHeap* ppHeaps[] = {descriptorHeap.Get()};
	commandList->SetDescriptorHeaps(_countof(ppHeaps),ppHeaps);

	//シェーダーリソースビュー(SRV)をセット
	commandList->SetGraphicsRootDescriptorTable(rootParamIndex, texture[textureHandle].gpuDescHandleSRV);
}
