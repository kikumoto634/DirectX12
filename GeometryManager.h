#pragma once

#include <DirectXMath.h>
#include <d3d12.h>
#include <wrl.h>

#include "DirectXCommon.h"
#include "TextureManager.h"

class GeometryManager
{
/// <summary>
/// エイリアス
/// </summary>
public:
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

/// <summary>
/// 定数
/// </summary>
public:
	static const int maxObjectCount = 512;

/// <summary>
/// インナークラス
/// </summary>
public:
	//頂点データ構造体(3D)
	struct Vertex
	{
		DirectX::XMFLOAT3 pos;	//xyz座標
		DirectX::XMFLOAT3 normal;//法線ベクトル
		DirectX::XMFLOAT2 uv;	//uv座標
	};

/// <summary>
/// メンバ関数
/// </summary>
public:

	void Initialize(DirectXCommon* dxCommon, TextureManager* textureManager);

	void Draw(ID3D12GraphicsCommandList* commandList, UINT texNumber);

	//getter
	const D3D12_VERTEX_BUFFER_VIEW& GetVbView() {return vbView;}
	const D3D12_INDEX_BUFFER_VIEW& GetIbView()	{return ibView;}

/// <summary>
/// メンバ変数
/// </summary>
private:
	//テクスチャマネージャー
	TextureManager* textureManager = nullptr;

	///頂点バッファ
	ComPtr<ID3D12Resource> vertBuff;
	///インデックスバッファ
	ComPtr<ID3D12Resource> indexBuff;

	///頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vbView{};
	///インデックスバッファビュー
	D3D12_INDEX_BUFFER_VIEW ibView{};

	/// <summary>
	/// 頂点データ
	/// </summary>
	Vertex vertices[24];
	uint16_t indices[36];
};

