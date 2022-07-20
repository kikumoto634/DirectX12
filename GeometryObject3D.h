#pragma once
#include <wrl.h>
#include <d3d12.h>

#include "DirectXCommon.h"
#include "GeometryModel.h"

/// <summary>
/// 数式で生成したモデル用3Dオブジェクト
/// </summary>
class GeometryObject3D
{
/// <summary>
/// エイリアス
/// </summary>
public:
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	using XMMATRIX = DirectX::XMMATRIX;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;

/// <summary>
/// 定数
/// </summary>
public:
	//3Dオブジェクトの最大数
	static const int maxObjectCount = 512;

/// <summary>
/// インナークラス
/// </summary>
public:

	//共通データ
	class Common
	{
		friend class GeometryObject3D;
		
	public:
		/// <summary>
		/// グラフィックスパイプライン初期化
		/// </summary>
		/// <param name="dxCommon">DirectX12ベース</param>
		void InitializeGraphicsPipeline(DirectXCommon* dxCommon);

		/// <summary>
		/// デスクリプタヒープ初期化
		/// </summary>
		/// <param name="dxCommon">DirectX12ベース</param>
		void InitializeDescriptorHeap(DirectXCommon* dxCommon);

		/// <summary>
		/// カメラの初期化
		/// </summary>
		void InitializeCamera();

	private:
		//DirectX12ベース
		DirectXCommon* dxCommon = nullptr;

		//パイプラインステートオブジェクト
		ComPtr<ID3D12PipelineState> pipelinestate;
		//ルートシグネチャ
		ComPtr<ID3D12RootSignature> rootsignature;
		//デスクリプタヒープ(定数バッファビュー用)
		ComPtr<ID3D12DescriptorHeap> basicDescHeap;
		//次に使うデスクリプタヒープの番号
		int descHeapIndex = 0;

		//モデルデータ
		GeometryModel* model = nullptr;

		///カメラ関連ここから
		//透視投影
		XMMATRIX matProjection;	//プロジェクション行列
		//ビュー変換行列
		float angle = 0.0f;		//カメラの回転角
		float distance = 100.f;	//カメラの距離
		XMMATRIX matView;		//ビュー行列
		XMFLOAT3 eye;			//視点座標
		XMFLOAT3 target;		//注視点座標
		XMFLOAT3 up;			//上方向ベクトル
		///ここまで
	};

	//定数バッファ用データ構造体(3D変換行列
	struct ConstBufferData{
		XMMATRIX mat;	//3D変換行列
		XMFLOAT4 color;	//色(RGBA)
	};

/// <summary>
/// 静的メンバ関数
/// </summary>
public:
	/// <summary>
	/// 初期化
	/// </summary>
	static void StaticInitialize(DirectXCommon* dxCommon, GeometryModel* model);

	/// <summary>
	/// デスクリプタヒープリセット
	/// </summary>
	static void ResetDescriptorHeap();

	/// <summary>
	/// 解放
	/// </summary>
	static void StaticFinalize();

/// <summary>
/// 静的メンバ変数
/// </summary>
private:
	static Common* common;

/// <summary>
/// メンバ関数
/// </summary>
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	void Update();

/// <summary>
/// メンバ変数
/// </summary>
private:

	//定数バッファ(行列用)
	ComPtr<ID3D12Resource> constBuff;
	//定数バッファビューのCPUアドレス
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescHandleCBV;
	//定数バッファビューのGPUアドレス
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescHandleCBV;

	//トランスフォーム
	//アフィン変換
	XMFLOAT3 scale = {1.0f, 1.0f, 1.0f};
	XMFLOAT3 rotation = {0.0f, 0.0f, 0.0f};
	XMFLOAT3 position = {0.0f, 0.0f, 0.0f};
	//ワールド変換行列
	XMMATRIX matWorld;
	//親オブジェクトへのポインタ
	GeometryObject3D* parent = nullptr;
};

