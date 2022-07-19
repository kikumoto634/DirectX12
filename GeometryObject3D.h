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
		void InitializeGraphicsPipeline(DirectXCommon* dxCommon);

	private:
		//パイプラインステートオブジェクト
		ComPtr<ID3D12PipelineState> pipelinestate;
		//ルートシグネチャ
		ComPtr<ID3D12RootSignature> rootsignature;

		//モデルデータ
		GeometryModel* model = nullptr;
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
	void Initialize();

private:
};

