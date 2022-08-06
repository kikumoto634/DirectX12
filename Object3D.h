#pragma once
#include "Model.h"
#include "Camera.h"

#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <DirectXMath.h>
#include <string>

#include "WorldTransform.h"
#include "ViewProjection.h"

#include "FbxLoader.h"

class Object3D
{
/// <summary>
/// 定数
/// </summary>
public:
	//ボーンの最大数
	static const int MAX_BONES = 32;

/// <summary>
/// エイリアス
/// </summary>
protected:
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMMATRIX = DirectX::XMMATRIX;

public:
	//サブクラス

	//定数バッファ用データ構造体(スキニング)
	struct ConstBufferDataSkin
	{
		XMMATRIX bones[MAX_BONES];
	};

/// <summary>
/// 静的メンバ関数
/// </summary>
public:
	//setter
	static void SetDevice(ID3D12Device* device) {Object3D::device = device; }
	/// <summary>
	/// グラフィックスパイプラインの生成
	/// </summary>
	static void CreateGraphicsPipeline();

/// <summary>
/// メンバ関数
/// </summary>
public:

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw(ID3D12GraphicsCommandList* commandList, const WorldTransform& worldTransform, const ViewProjection& viewProjection);

	/// <summary>
	/// モデルセット
	/// </summary>
	/// <param name="model">モデル</param>
	void SetModel(Model* model)	{this->model = model;}

	/// <summary>
	/// アニメーション開始
	/// </summary>
	void PlayAnimation();


	//setter
	void SetScale(XMFLOAT3 scale)	{this->scale = scale;}
	void SetRotation(XMFLOAT3 rotation)	{this->rotation = rotation;}
	void SetPosition(XMFLOAT3 position)	{this->position = position;}

	//getter
	XMFLOAT3 GetScale()	{return this->scale;}
	XMFLOAT3 GetRotation()	{return this->rotation;}
	XMFLOAT3 GetPosition()	{return this->position;}


/// <summary>
/// 静的メンバ変数
/// </summary>
private:
	static ID3D12Device* device;

	//ルートシグネチャ
	static ComPtr<ID3D12RootSignature> rootsignature;
	//パイプラインステートオブジェクト
	static ComPtr<ID3D12PipelineState> pipelinestate;

/// <summary>
/// メンバ変数
/// </summary>
private:

	//定数バッファ(スキン)
	ComPtr<ID3D12Resource> constBufferSkin;

	//ローカルスケール
	XMFLOAT3 scale = {1, 1, 1};
	//X,Y,Z軸周りのローカル回転角
	XMFLOAT3 rotation = {0, 0, 0};
	//ローカル座標
	XMFLOAT3 position = {0, 0, 0};
	//ローカルワールド変換行列
	XMMATRIX matWorld{};
	//モデル
	Model* model = nullptr;

	//1frameの時間
	FbxTime frameTime;
	//アニメーション開始時間
	FbxTime startTime;
	//アニメーション終了時間
	FbxTime endTime;
	//現在時間(アニメーション)
	FbxTime currentTime;
	//アニメーション再生中
	bool isPlay = false;
};

