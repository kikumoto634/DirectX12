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

	ComPtr<IDXGIFactory7> dxgiFactory= nullptr;
	ComPtr<ID3D12Device> device = nullptr;
	ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	ComPtr<IDXGISwapChain4> swapChain = nullptr;
	std::vector<ComPtr<ID3D12Resource>> backBuffers;
	ComPtr<ID3D12Resource> depthBuff = nullptr;
	ComPtr<ID3D12DescriptorHeap> dsvHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap> rtvHeap = nullptr;
	ComPtr<ID3D12Fence> fence;
	UINT64 fenceVal = 0;
};

