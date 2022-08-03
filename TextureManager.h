#pragma once

#include <d3d12.h>
#include <d3dx12.h>
#include <array>
#include <wrl.h>
#include "DirectXTex.h"

using namespace DirectX;

//テクスチャマネージャー
class TextureManager
{
private://テンプレ
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	using string = std::string;

public:	//定数
	static const size_t kNumDescriptors = 512;

public://サブクラス
	//テクスチャ
	struct Texture
	{
		//テクスチャリソース
		ComPtr<ID3D12Resource> resource;
		//シェーダーリソースビューハンドル
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescHandleSRV;
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescHandleSRV;
		//名前
		string name;
	};

public:	//静的メンバ関数

	/// <summary>
	/// 読み込み
	/// </summary>
	/// <param name="fileName">ファイル名</param>
	/// <returns>テクスチャハンドル</returns>
	static uint32_t Load(const string& fileName);

	/// <summary>
	/// シングルトンインスタンス
	/// </summary>
	/// <returns>シングルトンインスタンス</returns>
	static TextureManager* GetInstance();

public:	//メンバ関数

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(ID3D12Device* device, std::string directoryPath = "Resources/");

	/// <summary>
	/// テクスチャリセット
	/// </summary>
	void ResetAll();

	/// <summary>
	/// 読み込み
	/// </summary>
	/// <param name="fileName">ファイル名</param>
	/// <returns></returns>
	uint32_t LoadInternal(const string& fileName);

	/// <summary>
	/// リソース情報
	/// </summary>
	/// <param name="textureHandle">テクスチャハンドル</param>
	/// <returns>リソース</returns>
	const D3D12_RESOURCE_DESC GetResourceDesc(uint32_t textureHandle);

	/// <summary>
	/// デスクリプタヒープテーブル セット
	/// </summary>
	/// <param name="commandList">グラフィックスパイプライン</param>
	/// <param name="rootParamIndex">ルートパラメータ</param>
	/// <param name="textureHandle">テクスチャハンドル</param>
	void SetGraphicsRootDescriptorTable(ID3D12GraphicsCommandList* commandList, UINT rootParamIndex, uint32_t textureHandle);

private: //メンバ変数
	//デバイス
	ID3D12Device* device;
	//デスクリプタサイズ
	UINT descriptorHandleSize = 0u;
	//ディレクトリパス
	string directoryPath;
	//デスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> descriptorHeap;
	//デスクリプタヒープ番号
	uint32_t indexDescriptorHeap = 0u;
	//テクスチャコンテナ
	std::array<Texture, kNumDescriptors> texture;
};


