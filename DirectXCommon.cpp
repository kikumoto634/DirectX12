#include "DirectXCommon.h"

#include <cassert>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

void DirectXCommon::Initialize(WinApp* winApp)
{
	this->winApp = winApp;

	//DXGIデバイス初期化
	InitializeDXGIDevice();

	//コマンドリスト関連初期化
	InitializeCommand();

	//スワップチェーン生成
	CreateSwapChain();

	//レンダーターゲット生成
	CreateFinalRenderTargets();

	//深度バッファ生成
	CreateDepthBuffer();

	//フェンス生成
	CreateFence();
}

void DirectXCommon::BeginDraw()
{
	//リソースバリア01
	//バックバッファの番号を取得(ダブルバッファなので 0 or 1)
	UINT bbIndex = swapChain->GetCurrentBackBufferIndex();
		
	//1. リソースバリアで書き込み可能に変更
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
	  backBuffers[bbIndex].Get(), D3D12_RESOURCE_STATE_PRESENT,
	  D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->ResourceBarrier(1, &barrier);

	//2. 描画先の変更
	// レンダーターゲットビュー用ディスクリプタヒープのハンドルを取得
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvH = CD3DX12_CPU_DESCRIPTOR_HANDLE(
	  rtvHeap->GetCPUDescriptorHandleForHeapStart(), bbIndex,
	  device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
	// 深度ステンシルビュー用デスクリプタヒープのハンドルを取得
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvH =
	  CD3DX12_CPU_DESCRIPTOR_HANDLE(dsvHeap->GetCPUDescriptorHandleForHeapStart());
	//セット
	commandList->OMSetRenderTargets( 1, &rtvH, false, &dsvH );


	///画面クリアコマンド
	ClearRenderTarget();

	//深度クリア
	ClearDepthBuffer();

	///ビューポート
	//設定コマンド
	commandList->RSSetViewports(1, &CD3DX12_VIEWPORT(0.0f, 0.0f, WinApp::window_width,WinApp::window_height));
		 
	///シザー矩形
	//設定
	commandList->RSSetScissorRects(1, &CD3DX12_RECT(0, 0, WinApp::window_width, WinApp::window_height));

}

void DirectXCommon::EndDraw()
{
	HRESULT result = S_FALSE;

	UINT bbIndex = swapChain->GetCurrentBackBufferIndex();


	//5. リソースバリアを戻す
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
	  backBuffers[bbIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET,
	  D3D12_RESOURCE_STATE_PRESENT);
	commandList->ResourceBarrier(1, &barrier);


	///コマンドのフラッシュ
	//命令クローズ
	commandList->Close();
	//コマンドリストの実行
	ID3D12CommandList* commandLists[] = {commandList.Get()};
	commandQueue->ExecuteCommandLists(1, commandLists);
		

	//画面に表示するバッファをフリップ(表裏の入れ替え)
	result = swapChain->Present(1, 0);
#ifdef _DEBUG

	if(FAILED(result))
	{
		ComPtr<ID3D12DeviceRemovedExtendedData> dred;
		
		result = device->QueryInterface(IID_PPV_ARGS(&dred));
		assert(SUCCEEDED(result));

		//自動パンくず取得
		D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT autoBreadcrumbsOutput{};
		result = dred->GetAutoBreadcrumbsOutput(&autoBreadcrumbsOutput);
		assert(SUCCEEDED(result));
	}

#endif // _DEBUG


	///コマンド完了待ち
	//コマンドの実行完了を待つ
	commandQueue->Signal(fence.Get(), ++fenceVal);
	if(fence->GetCompletedValue() != fenceVal)
	{
		HANDLE event = CreateEvent(nullptr, false, false, nullptr);
		fence->SetEventOnCompletion(fenceVal, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}

	//キュークリア
	commandAllocator->Reset();
	//再びコマンドリストを溜める準備
	commandList->Reset(commandAllocator.Get(), nullptr);
}

void DirectXCommon::ClearRenderTarget()
{
	//バックバッファの番号を取得(ダブルバッファなので 0 or 1)
	UINT bbIndex = swapChain->GetCurrentBackBufferIndex();

	//3. 画面クリア
	FLOAT clearColor[] = {0.1f, 0.25f, 0.5f, 0.0f};
	
	// レンダーターゲットビュー用ディスクリプタヒープのハンドルを取得
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvH = CD3DX12_CPU_DESCRIPTOR_HANDLE(
	  rtvHeap->GetCPUDescriptorHandleForHeapStart(), bbIndex,
	  device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));

	//色クリア
	commandList->ClearRenderTargetView( rtvH, clearColor, 0, nullptr );
}

void DirectXCommon::ClearDepthBuffer()
{
	// 深度ステンシルビュー用デスクリプタヒープのハンドルを取得
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvH =
	  CD3DX12_CPU_DESCRIPTOR_HANDLE(dsvHeap->GetCPUDescriptorHandleForHeapStart());
	// 深度バッファのクリア
	commandList->ClearDepthStencilView( dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr );
}

void DirectXCommon::InitializeDXGIDevice()
{
	HRESULT result;

	///デバックレイヤー
#ifdef _DEBUG
	ComPtr<ID3D12Debug> debugController;
	//デバックレイヤーをオンに
	if(SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))){
		debugController->EnableDebugLayer();
	}
	//DREDレポートをオンに
	ComPtr<ID3D12DeviceRemovedExtendedDataSettings> dredSettings;
	if(SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&dredSettings))))
	{
		dredSettings->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
		dredSettings->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
	}
#endif // _DEBUG

	///デバイスの生成(1ゲームに一つ)
	//対応レベルの配列
	D3D_FEATURE_LEVEL levels[] = 
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	///アダプタ列挙
	//DXGIファクトリー
	result = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	assert(SUCCEEDED(result));

	//アダプターの列挙用
	std::vector<ComPtr<IDXGIAdapter4>> adapters;
	//ここに特定の名前を持つアダプターオブジェクトが入る
	ComPtr<IDXGIAdapter4> tmpAdapter;

	//パフォーマンスが高いものから順に、すべてのアダプタを列挙する
	for(UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&tmpAdapter)) != DXGI_ERROR_NOT_FOUND; i++)
	{
		//動的配列に追加する
		adapters.push_back(tmpAdapter);
	}

	///アダプタの選別
	//打倒なアダプタを選別する
	for(size_t i = 0; i < adapters.size(); i++)
	{
		DXGI_ADAPTER_DESC3 adapterDesc;
		//アダプターの情報を取得する
		adapters[i]->GetDesc3(&adapterDesc);

		//ソフトウェアデバイスを回避
		if(!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE))
		{
			//デバイスを採用してループを抜ける
			tmpAdapter = adapters[i].Get();
			break;
		}
	}

	D3D_FEATURE_LEVEL featureLevel;
	for(size_t i = 0; i < _countof(levels); i++)
	{
		//採用したアダプターでデバイスを生成
		result = D3D12CreateDevice(tmpAdapter.Get(), levels[i], IID_PPV_ARGS(&device));
		if(result == S_OK)
		{
			//デバイスを生成出来た時点でループを抜ける
			featureLevel = levels[i];
			break;
		}
	}
	assert(SUCCEEDED(result));

}

void DirectXCommon::CreateSwapChain()
{
	HRESULT result = S_FALSE;

	///スワップチェーン(フロントバッファ、バックバッファを入れ替えてパラパラ漫画を作る)
	//設定
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = WinApp::window_width;
	swapChainDesc.Height = WinApp::window_height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;				//色情報書式
	swapChainDesc.SampleDesc.Count = 1;								//マルチサンプルしない
	swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;				//バックバッファ用
	swapChainDesc.BufferCount = 2;									//バッファ数を二つに設定
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;		//フリップ後は破棄
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	//生成
	//IDXGISwapChainのComPtr用意
	ComPtr<IDXGISwapChain1> swapchain;
	result = dxgiFactory->CreateSwapChainForHwnd(
		commandQueue.Get(),
		this->winApp->GetHwnd(),
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapchain
	);
	assert(SUCCEEDED(result));


	//生成したIDXGISwapChain1のオブジェクトをIDXGISwapChain4に変換する
	// SwapChain4を得る
	swapchain->QueryInterface(IID_PPV_ARGS(&swapChain));
	assert(SUCCEEDED(result));

	// OSが行うAlt+Enterのフルスクリーンは制御不能なので禁止
	dxgiFactory->MakeWindowAssociation(
	  winApp->GetHwnd(), DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);
}

void DirectXCommon::InitializeCommand()
{
	HRESULT result = S_FALSE;

	///コマンドリスト(GPUに、まとめて命令を送るためのコマンド)
	//コマンドアロケータを生成
	result = device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&commandAllocator)
	);
	if(FAILED(result))
	{
		assert(0);
	}

	//コマンドリストを生成
	result = device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		commandAllocator.Get(),
		nullptr,
		IID_PPV_ARGS(&commandList)
	);
	if(FAILED(result))
	{
		assert(0);
	}


	///コマンドキュー(コマンドリストをGPUに順位実行させていく仕組み)
	//コマンドキューの設定
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	//コマンドキューを生成
	result = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	if(FAILED(result))
	{
		assert(0);
	}
}

void DirectXCommon::CreateFinalRenderTargets()
{
	HRESULT result = S_FALSE;

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	result = swapChain->GetDesc(&swapChainDesc);
	assert(SUCCEEDED(result));

	///レンダーターゲットビュー(バックバッファを描画キャンバスとして扱うオブジェクト)
	//デスクリプタヒープ生成(レンダーターゲットビューはデスクリプタヒープに生成するので準備)
	//設定
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;		//レンダーターゲットビュー
	rtvHeapDesc.NumDescriptors = swapChainDesc.BufferCount;	//表裏の二つ(ダブルバッファリング)
	//生成
	device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));

	//バックバッファ(スワップチェーン内で生成されたバックバッファのアドレス収容用)
	backBuffers.resize(swapChainDesc.BufferCount);

	//レンダーターゲットビュー(RTV)生成
	//スワップチェーンのすべてのバッファについて処理する
	for(size_t i = 0; i < backBuffers.size(); i++)
	{
		//スワップチェーンからバッファを取得
		swapChain->GetBuffer((UINT)i, IID_PPV_ARGS(&backBuffers[i]));
		//レンダーターゲットビューの設定
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		//シェーダーの計算結果をSRGBに変換して書き込む
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		//レンダーターゲットビューの生成
		device->CreateRenderTargetView
		(
			backBuffers[i].Get(),
			&rtvDesc,
			CD3DX12_CPU_DESCRIPTOR_HANDLE//デスクリプタヒープのハンドルを取得
			(//表か裏でアドレスがずれる
				rtvHeap->GetCPUDescriptorHandleForHeapStart(),
				INT(i),
				device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type)
			)
		);
	}
}

void DirectXCommon::CreateDepthBuffer()
{
	HRESULT result = S_FALSE;

	///深度バッファのリソース(テクスチャの一種)
	//リソース設定
	CD3DX12_RESOURCE_DESC depthReourceDesc = CD3DX12_RESOURCE_DESC::Tex2D
		(
			DXGI_FORMAT_D32_FLOAT,
			WinApp::window_width,
			WinApp::window_height,
			1,0,
			1,0,
			D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
		);
	//深度バッファの生成
	result = device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&depthReourceDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,	//深度値書き込みに使用
			&CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0),
			IID_PPV_ARGS(&depthBuff)
		);
	if(FAILED(result))
	{
		assert(0);
	}
	
	//深度ビュー用デスクリプタヒープ作成(DSV)
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
	dsvHeapDesc.NumDescriptors = 1;			//深度ビューは一つ
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;	//デプスステンシルビュー
	result = device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap));
	if(FAILED(result))
	{
		assert(0);
	}
	//深度ビュー作成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	device->CreateDepthStencilView(
		depthBuff.Get(),
		&dsvDesc,
		dsvHeap->GetCPUDescriptorHandleForHeapStart()
	);
}

void DirectXCommon::CreateFence()
{
	HRESULT result = S_FALSE;

	///フェンス(CPUとGPUで同期をとるための仕組み)
	//生成
	result = device->CreateFence(fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	if(FAILED(result))
	{
		assert(0);
	}
}
