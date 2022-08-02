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
	
	//ローカル
	DirectX::XMFLOAT3 scale = {1, 1, 1};
	DirectX::XMFLOAT3 rotation = {0, 0, 0};
	DirectX::XMFLOAT3 translation = {0, 0, 0};
	//ワールド
	DirectX::XMMATRIX matWorld;
	//親
	WorldTransform*parent = nullptr;

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
	/// 行列を更新
	/// </summary>
	void UpdateMatrix();
};

