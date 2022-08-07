#pragma once
#include "FbxModelMesh.h"
#include "WorldTransform.h"
#include "ViewProjection.h"

#include <wrl.h>
#include <DirectXMath.h>
#include <string>
#include <vector>

/// <summary>
/// Fbxモデル
/// </summary>
class FbxModel
{
private:	//エイリアス
	template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMMATRIX = DirectX::XMMATRIX;
	using string = std::string;

public:	//列挙
	enum class RootParameter
	{
		WorldTransform,
		ViewProjection,
		Texture,
	};

public:	//定数
	static const int MAX_BONES = 32;

private://静的定数
	static const string kBaseDirectory;

public:	//サブクラス
	//定数バッファ用データ構造(スキニング)
	struct ConstBufferDataSkin
	{
		XMMATRIX bones[MAX_BONES];
	};

private://静的メンバ関数

	static void StaticInitialize();

	static void InitializeGraphicsPipeline();

	static FbxModelMesh* Create();

	static void PreDraw();

	static void PostDraw();

public:	//メンバ関数
	
	~FbxModel();

	void Initialize();

	void Draw(const WorldTransform& worldTransform, const ViewProjection& viewProjection);

	void SetModel();

	void PlayAnimation();

private:	//メンバ変数

	ComPtr<ID3D12Resource> constBufferSkin;

	FbxTime frameTime;
	

	std::vector<FbxModelMesh> mesh;
};

