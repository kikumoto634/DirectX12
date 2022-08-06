#pragma once

#include <DirectXMath.h>
#include <d3d12.h>
#include <wrl.h>

///定数バッファ用
struct ConstBufferDataViewProjection
{
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX projection;
	DirectX::XMFLOAT3 cameraPos;
};

/// <summary>
/// ビュープロジェクション
/// </summary>
struct ViewProjection
{
	//定数バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> constBuff;
	//マッピング
	ConstBufferDataViewProjection* constMap = nullptr;

#pragma region ビュー行列
	//視点座標
	DirectX::XMFLOAT3 eye = {0, 0, -50.f};
	//注視点座標
	DirectX::XMFLOAT3 target = {0,0,0};
	//上方向ベクトル
	DirectX::XMFLOAT3 up = {0, 1, 0};
#pragma endregion

#pragma region	射影変換行列
	//視野角
	float fovAngle = DirectX::XMConvertToRadians(45.f);
	//アスペクト比
	float aspectRatio = (float)16/9;
	//深度
	float nearZ = 0.1f;
	float farZ = 10000.f;
#pragma endregion

	//ビュー
	DirectX::XMMATRIX matView;
	//射影
	DirectX::XMMATRIX matProjection;

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

