#pragma once

#include <d3dx12.h>
#include <wrl.h>
#include <string>
#include <array>

//テクスチャマネージャー
class TextureManager
{
public://エイリアス
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

public:	//定数
	//テクスチャの最大枚数
	static const size_t kNumDescriptors = 256;

public://サブクラス
	struct Texture
	{
		//テクスチャリソース
		ComPtr<ID3D12Resource> resource;
		//シェーダーリソースビューのハンドル(CPU)
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescHandleSRV;
		//シェーダーリソースビューのハンドル(GPU)
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDeschandleSRV;
		//名前
		std::string name;
	};

public:	//メンバ関数

	static uint32_t Load(const std::string& fileName);

	static TextureManager* GetInstance();

	void Initialize(ID3D12Device* device, std::string directoryPath = "Resources/");

	void ResetAll();

	const D3D12_RESOURCE_DESC GetResourceDesc(uint32_t texturehandle);

	void SetGraphicsRootDescriptorTable(ID3D12GraphicsCommandList*commandList, UINT rootParamIndex,uint32_t textureHandle);

private://メンバ変数

	ID3D12Device* device;
	UINT descriptorHandleIncrementSize = 0u;
	std::string directoryPath;
	ComPtr<ID3D12DescriptorHeap> desciptorHeap;
	uint32_t indexNextDescriptorHeap = 0u;
	std::array<Texture, kNumDescriptors> textures;

	uint32_t LoadInternal(const std::string& fileName);
};


