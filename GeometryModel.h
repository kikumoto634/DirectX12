#pragma once
#include <DirectXMath.h>
#include <d3d12.h>

#include "DirectXCommon.h"

/// <summary>
/// 幾何学モデル
/// </summary>
class GeometryModel
{
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

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon);
};

