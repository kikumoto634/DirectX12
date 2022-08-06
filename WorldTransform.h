#pragma once
#include <DirectXMath.h>
#include <d3d12.h>
#include <wrl.h>

//定数バッファ
struct ConstBufferDataWorldTransform
{
	DirectX::XMMATRIX matWorld;
};

/// <summary>
/// ワールド変換
/// </summary>
struct WorldTransform
{
	//定数バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> constBuff;
	//マッピング
	ConstBufferDataWorldTransform* constMap = nullptr;
	//スケール
	DirectX::XMFLOAT3 scale = {1,1,1};
	//回転角
	DirectX::XMFLOAT3 rotation = {0,0,0};
	//座標
	DirectX::XMFLOAT3 position = {0,0,0};
	//ワールド
	DirectX::XMMATRIX matWorld;
	//親
	WorldTransform* parent = nullptr;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 定数バッファ生成
	/// </summary>
	void CreateConstBuffer();

	/// <summary>
	/// マッピング
	/// </summary>
	void Map();

	/// <summary>
	/// 行列更新
	/// </summary>
	void UpdateMatrix();
};

