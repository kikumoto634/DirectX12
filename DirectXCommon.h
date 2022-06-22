#pragma once
#include "WinApp.h"

//基本(初期化)
#include <d3d12.h>
#include <dxgi1_6.h>

//アダプタの列挙
#include <vector>
#include <string>

//ComPtrスマートポインタ
#include <wrl.h>

//CD3DX12ヘルパー構造体
#include <d3dx12.h>


/// <summary>
/// DIrectX
/// </summary>
class DirectXCommon
{
public://エイリアス
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

public://メンバ関数
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(WinApp* winApp);

	/// <summary>
	/// 毎フレーム処理開始
	/// </summary>
	void BeginDraw();

	/// <summary>
	/// 毎フレーム処理終了
	/// </summary>
	void EndDraw();

	/// <summary>
	/// device取得
	/// </summary>
	/// <returns></returns>
	ID3D12Device* GetDevice() { return device.Get(); }

	ID3D12GraphicsCommandList* GetCommandList(){ return commandList.Get(); }

private://メンバ変数
	WinApp* winApp = nullptr;

	//DXGIファクトリー
	ComPtr<IDXGIFactory7> dxgiFactory= nullptr;
	//デバイス
	ComPtr<ID3D12Device> device = nullptr;
	//グラフィックスコマンドリスト
	ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	//コマンドアロケータ
	ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	//コマンドキュー
	ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	//スワップチェーン
	ComPtr<IDXGISwapChain4> swapChain = nullptr;
	//バックバッファ
	std::vector<ComPtr<ID3D12Resource>> backBuffers;
	//深度バッファ
	ComPtr<ID3D12Resource> depthBuff = nullptr;
	//深度ステンシルビュー
	ComPtr<ID3D12DescriptorHeap> dsvHeap = nullptr;
	//レンダーターゲットビュー
	ComPtr<ID3D12DescriptorHeap> rtvHeap = nullptr;
	//フェンス
	ComPtr<ID3D12Fence> fence;
	UINT64 fenceVal = 0;
};

