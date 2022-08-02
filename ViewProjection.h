#pragma once
#include <DirectXMath.h>
#include <d3d12.h>
#include <wrl.h>

//定数バッファ用データ構造体
struct ConstBufferViewProjection
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
	//マッピング済み
	ConstBufferViewProjection* constMap = nullptr;

#pragma region ビュー行列の設定

	//視点座標
	DirectX::XMFLOAT3 eye = {0, 0, -50.f};
	//注視点座標
	DirectX::XMFLOAT3 target = {0, 0, 0};
	//上方向ベクトル
	DirectX::XMFLOAT3 up = {0, 1, 0};

#pragma endregion


#pragma region 射影行列の設定

	//水平方向視野角
	float fovAngleY = DirectX::XMConvertToRadians(45.f);
	//ビューポートのアスペクト
	float aspectRatio = (float)16/ 9;
	//深度限界
	float nearZ = 0.1f;
	float farZ = 1000.f;

#pragma endregion

	//ビュー行列
	DirectX::XMMATRIX matView;
	//射影行列
	DirectX::XMMATRIX matProjection;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 定数バッファの生成
	/// </summary>
	void CreateConstBuffer();

	/// <summary>
	/// マッピングする
	/// </summary>
	void Map();

	/// <summary>
	/// 行列を更新する
	/// </summary>
	void UpdateMatrix();
};

