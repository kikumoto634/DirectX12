#pragma once

#include <d3d12.h>
//CD3DX12ヘルパー構造体
#include <d3dx12.h>
//ComPtrスマートポインタ
#include <wrl.h>
//DirectXTex導入
#include "DirectXTex.h"


#include "DirectXCommon.h"

/// <summary>
/// テクスチャマネージャー
/// </summary>
class TextureManager
{
public://エイリアス
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

public://定数
	//SRVの最大個数
	static const int spriteSRVCount = 2056;

public://メンバ関数
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon"></param>
	void Inithalize(DirectXCommon* dxCommon);

	/// <summary>
	/// テクスチャ読込
	/// </summary>
	/// <param name="texnumber">テクスチャ番号</param>
	/// <param name="filename">テクスチャファイル名</param>
	void LoadTexture(UINT texnumber, const wchar_t* filename);

	/// <summary>
	/// テクスチャバッファ番号取得
	/// </summary>
	/// <param name="texnumber"></param>
	/// <returns></returns>
	ID3D12Resource* GetSpriteTexBuffer(UINT texnumber);

	/// <summary>
	/// デスクリプタヒープをセット
	/// </summary>
	/// <param name="coomandList">グラフィックスコマンド</param>
	void SetDescriptorHeaps(ID3D12GraphicsCommandList* commandList);

	/// <summary>
	/// SRVをセット(グラフィックスコマンド)
	/// </summary>
	/// <param name="commandList">コマンドリスト</param>
	/// <param name="rootParameterIndex">ルートパラメータ番号</param>
	/// <param name="texnumber">テクスチャ番号</param>
	void SetShaderResourceView(ID3D12GraphicsCommandList* commandList, UINT rootParameterIndex,UINT texnumber);

	//getter
	ID3D12DescriptorHeap* GetSpriteDescHeap() {return spriteDescHeap.Get(); }

private://メンバ変数
	ComPtr<ID3D12DescriptorHeap> spriteDescHeap = nullptr;
	//テクスチャバッファの生成
	ComPtr<ID3D12Resource> spriteTexBuff[spriteSRVCount];

	DirectXCommon* dxCommon = nullptr;
};

