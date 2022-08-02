#pragma once
#include "WinApp.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>
#include <string>
#include <wrl.h>
#include <d3dx12.h>


/// <summary>
/// DirectX
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
	/// レンダーターゲットクリア
	/// </summary>
	void ClearRenderTarget();

	/// <summary>
	/// 深度バッファのクリア
	/// </summary>
	void ClearDepthBuffer();


	/// <summary>
	/// device取得
	/// </summary>
	/// <returns></returns>
	ID3D12Device* GetDevice() { return device.Get(); }

	ID3D12GraphicsCommandList* GetCommandList(){ return commandList.Get(); }

private://メンバ変数
	WinApp* winApp = nullptr;

#pragma region DirectX3D関連

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

#pragma endregion DirectX3D関連


private://メンバ関数

	/// <summary>
	/// DXGIデバイス初期化
	/// </summary>
	void InitializeDXGIDevice();

	/// <summary>
	/// スワップチェーンの生成
	/// </summary>
	void CreateSwapChain();

	/// <summary>
	/// コマンド関連初期化
	/// </summary>
	void InitializeCommand();

	/// <summary>
	/// レンダーターゲット生成
	/// </summary>
	void CreateFinalRenderTargets();

	/// <summary>
	/// 深度バッファ生成
	/// </summary>
	void CreateDepthBuffer();

	/// <summary>
	/// フェンス生成
	/// </summary>
	void CreateFence();
};

