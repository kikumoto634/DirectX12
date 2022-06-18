#include <Windows.h>

//基本(初期化)
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

//アダプタの列挙
#include <vector>
#include <string>

//図形描画
#include <DirectXMath.h>
using namespace DirectX;

//D3Dコンパイラのインクルード
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

//キーボード入力
#define DIRECTINPUT_VERSION 0x0800	//DirectInputのバージョン指定
#include <dinput.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

//DirectXTex導入
#include "DirectXTex.h"

//ComPtrスマートポインタ
#include <wrl.h>
using namespace Microsoft::WRL;


//頂点データ構造体
struct Vertex
{
	XMFLOAT3 pos;	//xyz座標
	XMFLOAT3 normal;//法線ベクトル
	XMFLOAT2 uv;	//uv座標
};

//定数バッファ用データ構造体(マテリアル)
struct ConstBufferDataMaterial{
	XMFLOAT4 color;	//色(RGBA)
};

//定数バッファ用データ構造体(3D変換行列
struct ConstBufferDataTransform{
	XMMATRIX mat;	//3D変換行列
};

//3Dオブジェクト型
struct Object3d
{
	//定数バッファ(行列用)
	ComPtr<ID3D12Resource> constBuffTransform;
	//定数バッファマップ(行列用)
	ConstBufferDataTransform* constMapTransform;
	//アフィン変換
	XMFLOAT3 scale = {1.0f, 1.0f, 1.0f};
	XMFLOAT3 rotation = {0.0f, 0.0f, 0.0f};
	XMFLOAT3 position = {0.0f, 0.0f, 0.0f};
	//ワールド変換行列
	XMMATRIX matWorld;
	//親オブジェクトへのポインタ
	Object3d* parent = nullptr;
};



/// <summary>
/// 入力
/// </summary>
void InputUpdate(IDirectInputDevice8* devkeyboard, BYTE key[], BYTE oldkey[], int arraysize);
bool Input(const BYTE key[], int KeysName);
bool Output(const BYTE key[], int KeysName);
bool IsInKeyTrigger(const BYTE key[], const BYTE oldkey[], int KeysName);
bool IsOutKeyTrigger(const BYTE key[], const BYTE oldkey[], int KeysName);

//3Dオブジェクト初期化
void InitializeObject3d(Object3d* object, ID3D12Device* device);
void UpdateObject3d(Object3d* object, XMMATRIX& matView, XMMATRIX& matProjection);
void DrawObject3d(Object3d* object, ID3D12GraphicsCommandList* commandList, D3D12_VERTEX_BUFFER_VIEW& vbView, D3D12_INDEX_BUFFER_VIEW& ibView, UINT numIndices);

/// ウィンドウプロシージャ
LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	//メッセージに応じてゲーム固有の処理を行う
	switch (msg){
		//ウィンドウが破棄された
	case WM_DESTROY:
		//OSに対して、アプリの終了を伝える
		PostQuitMessage(0);
		return 0;
	}

	//標準のメッセージ処理を行う
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

/// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE,LPSTR,int)
{
	//ウィンドウサイズ
	const int window_width = 1280;	//横幅
	const int window_height = 720;	//縦幅

	//ウィンドウクラス設定
	WNDCLASSEX w{};
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProc;		//ウィンドウプロシージャ設定
	w.lpszClassName = L"DirectXGame";			//ウィンドウクラス名
	w.hInstance = GetModuleHandle(nullptr);		//ウィンドウハンドル
	w.hCursor = LoadCursor(NULL, IDC_ARROW);	//カーソル指定

	//ウィンドウクラスをOSに登録する
	RegisterClassEx(&w);
	//ウィンドウサイズ{ X座標 Y座標 横幅 縦幅}
	RECT wrc = {0, 0, window_width, window_height};
	//自動でサイズを補正する
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	///ウィンドウオブジェクトの生成
	HWND hwnd = CreateWindow(
		w.lpszClassName,			//クラス名
		L"DirectXGame",				//タイトルバー文字
		WS_OVERLAPPEDWINDOW,		//標準的なウィンドウスタイル
		CW_USEDEFAULT,				//表示X座標(OSに任せる)
		CW_USEDEFAULT,				//表示Y座標(OSに任せる)
		wrc.right  - wrc.left,		//ウィンドウ横幅
		wrc.bottom - wrc.top,		//ウィンドウ縦幅
		nullptr,					//親ウィンドウハンドル
		nullptr,					//メニューハンドル
		w.hInstance,				//呼び出しアプリケーションハンドル
		nullptr						//オプション
	);
	//ウィンドウを表示状態にする
	ShowWindow(hwnd, SW_SHOW);

	MSG msg{};	//メッセージ


	/// <summary>
	/// DirectX12 初期化処理 ここから
	/// </summary>

	///デバックレイヤー
#ifdef _DEBUG
	//デバックレイヤーをオンに
	ID3D12Debug* debugController= nullptr;
	if(SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))){
		debugController->EnableDebugLayer();
	}
#endif // _DEBUG


	/// <summary>
	/// 初期化変数
	/// </summary>
	HRESULT result;
	ComPtr<ID3D12Device> device = nullptr;
	ComPtr<IDXGIFactory7> dxgiFactory= nullptr;
	ComPtr<IDXGISwapChain4> swapChain = nullptr;
	ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	ComPtr<ID3D12DescriptorHeap> rtvHeap = nullptr;

	///アダプタ列挙
	//DXGIファクトリー
	result = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	assert(SUCCEEDED(result));

	//アダプターの列挙用
	std::vector<ComPtr<IDXGIAdapter4>> adapters;
	//ここに特定の名前を持つアダプターオブジェクトが入る
	ComPtr<IDXGIAdapter4> tmpAdapter = nullptr;

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

	///デバイスの生成(1ゲームに一つ)
	//対応レベルの配列
	D3D_FEATURE_LEVEL levels[] = 
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	D3D_FEATURE_LEVEL featureLevel;
	for(size_t i = 0; i < _countof(levels); i++)
	{
		//採用したアダプターでデバイスを生成
		result = D3D12CreateDevice(tmpAdapter.Get(), levels[i], IID_PPV_ARGS(&device));
		assert(SUCCEEDED(result));
		if(result == S_OK)
		{
			//デバイスを生成出来た時点でループを抜ける
			featureLevel = levels[i];
			break;
		}
	}

	///コマンドリスト(GPUに、まとめて命令を送るためのコマンド)
	//コマンドアロケータを生成
	result = device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&commandAllocator)
	);
	assert(SUCCEEDED(result));

	//コマンドリストを生成
	result = device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		commandAllocator.Get(),
		nullptr,
		IID_PPV_ARGS(&commandList)
	);
	assert(SUCCEEDED(result));


	///コマンドキュー(コマンドリストをGPUに順位実行させていく仕組み)
	//コマンドキューの設定
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	//コマンドキューを生成
	result = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	assert(SUCCEEDED(result));


	///スワップチェーン(フロントバッファ、バックバッファを入れ替えてパラパラ漫画を作る)
	//設定
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = window_width;
	swapChainDesc.Height = window_height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;				//色情報書式
	swapChainDesc.SampleDesc.Count = 1;								//マルチサンプルしない
	swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;				//バックバッファ用
	swapChainDesc.BufferCount = 2;									//バッファ数を二つに設定
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;		//フリップ後は破棄
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	//生成
	//IDXGISwapChain1のComPtr用意
	ComPtr<IDXGISwapChain1> swapchain1;
	result = dxgiFactory->CreateSwapChainForHwnd(
		commandQueue.Get(),
		hwnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapchain1
	);
	assert(SUCCEEDED(result));
	//生成したIDXGISwapChain1のオブジェクトをIDXGISwapChain4に変換する
	swapchain1.As(&swapChain);

	///レンダーターゲットビュー(バックバッファを描画キャンバスとして扱うオブジェクト)
	//デスクリプタヒープ生成(レンダーターゲットビューはデスクリプタヒープに生成するので準備)
	//設定
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;		//レンダーターゲットビュー
	rtvHeapDesc.NumDescriptors = swapChainDesc.BufferCount;	//表裏の二つ(ダブルバッファリング)
	//生成
	device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));

	//バックバッファ(スワップチェーン内で生成されたバックバッファのアドレス収容用)
	std::vector<ComPtr<ID3D12Resource>> backBuffers;
	backBuffers.resize(swapChainDesc.BufferCount);

	//レンダーターゲットビュー(RTV)生成
	//スワップチェーンのすべてのバッファについて処理する
	for(size_t i = 0; i < backBuffers.size(); i++)
	{
		//スワップチェーンからバッファを取得
		swapChain->GetBuffer((UINT)i, IID_PPV_ARGS(&backBuffers[i]));
		//デスクリプタヒープのハンドルを取得
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
		//表か裏でアドレスがずれる
		rtvHandle.ptr += i * device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);
		//レンダーターゲットビューの設定
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		//シェーダーの計算結果をSRGBに変換して書き込む
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		//レンダーターゲットビューの生成
		device->CreateRenderTargetView(backBuffers[i].Get(), &rtvDesc, rtvHandle);
	}

	///深度バッファのリソース(テクスチャの一種)
	//設定
	D3D12_RESOURCE_DESC depthResourceDesc{};
	depthResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResourceDesc.Width = window_width;		//レンダーターゲットに合わせる
	depthResourceDesc.Height = window_height;	//レンダーターゲットに合わせる
	depthResourceDesc.DepthOrArraySize = 1;
	depthResourceDesc.Format = DXGI_FORMAT_D32_FLOAT;	//深度値フォーマット
	depthResourceDesc.SampleDesc.Count = 1;
	depthResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;	//デプスステンシル
	//深度値用ヒーププロパティ
	D3D12_HEAP_PROPERTIES depthHeapProp{};
	depthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	//深度値のクリア設定
	D3D12_CLEAR_VALUE idepthClearValue{};
	idepthClearValue.DepthStencil.Depth = 1.0f;		//深度値1.0f(最大値)でクリア
	idepthClearValue.Format = DXGI_FORMAT_D32_FLOAT;//深度値フォーマット
	//リソース生成
	ComPtr<ID3D12Resource> depthBuff;
	result = device->CreateCommittedResource(
		&depthHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&depthResourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&idepthClearValue,
		IID_PPV_ARGS(&depthBuff)
	);
	assert(SUCCEEDED(result));
	//深度ビュー用デスクリプタヒープ作成(DSV)
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
	dsvHeapDesc.NumDescriptors = 1;			//深度ビューは一つ
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;	//デプスステンシルビュー
	ComPtr<ID3D12DescriptorHeap> dsvHeap = nullptr;
	result = device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap));
	assert(SUCCEEDED(result));
	//深度ビュー作成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	device->CreateDepthStencilView(
		depthBuff.Get(),
		&dsvDesc,
		dsvHeap->GetCPUDescriptorHandleForHeapStart()
	);


	///フェンス(CPUとGPUで同期をとるための仕組み)
	//生成
	ComPtr<ID3D12Fence> fence;
	UINT64 fenceVal = 0;
	result = device->CreateFence(fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(result));

	///DirectInPut
	//初期化 (他入力方法追加でもこのオブジェクトは一つのみ)
	IDirectInput8* directInput = nullptr;
	result = DirectInput8Create(
		w.hInstance, 
		DIRECTINPUT_VERSION, 
		IID_IDirectInput8,
		(void**)&directInput, 
		nullptr
	);
	assert(SUCCEEDED(result));

	//キーボードデバイスの生成 (GUID_Joystick (ジョイステック)、 GUID_SysMouse (マウス))
	IDirectInputDevice8* keyboard = nullptr;
	result = directInput->CreateDevice(
		GUID_SysKeyboard,
		&keyboard,
		NULL
	);
	assert(SUCCEEDED(result));

	//入力データ形式のセット (入力デバイスの種類によって、あらかじめ何種類か用意する)
	result = keyboard->SetDataFormat(&c_dfDIKeyboard);	//標準形式
	assert(SUCCEEDED(result));

	//排他的制御レベルのセット
	//DISCL_FOREGROUND		画面が手前にある場合のみ入力を受け付ける
	//DISCL_NONEXCLUSIVE	デバイスをこのアプリだけで専有しない
	//DISCL_NOWINKEY		Windowsキーを無効にする
	result = keyboard->SetCooperativeLevel(
		hwnd,
		DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY
	);
	assert(SUCCEEDED(result));




	/// <summary>
	/// DirectX12 初期化処理 ここまで
	/// </summary>
	

	
	/// <summary>
	/// DirectX12 描画初期化処理 ここから
	/// </summary>
	 
	///頂点データ
	Vertex vertices[] = 
	{
		//前
		{{-5.0f, -5.0f, -5.0f}, {}, {0.0f, 1.0f}},
		{{-5.0f, +5.0f, -5.0f}, {}, {0.0f, 0.0f}},
		{{+5.0f, -5.0f, -5.0f}, {}, {1.0f, 1.0f}},
		{{+5.0f, +5.0f, -5.0f}, {}, {1.0f, 0.0f}},
		//後
		{{-5.0f, -5.0f, +5.0f}, {}, {0.0f, 1.0f}},
		{{-5.0f, +5.0f, +5.0f}, {}, {0.0f, 0.0f}},
		{{+5.0f, -5.0f, +5.0f}, {}, {1.0f, 1.0f}},
		{{+5.0f, +5.0f, +5.0f}, {}, {1.0f, 0.0f}},
		//左
		{{-5.0f, -5.0f, -5.0f}, {}, {0.0f, 1.0f}},
		{{-5.0f, -5.0f, +5.0f}, {}, {0.0f, 0.0f}},
		{{-5.0f, +5.0f, -5.0f}, {}, {1.0f, 1.0f}},
		{{-5.0f, +5.0f, +5.0f}, {}, {1.0f, 0.0f}},
		//右
		{{+5.0f, -5.0f, -5.0f}, {}, {0.0f, 1.0f}},
		{{+5.0f, -5.0f, +5.0f}, {}, {0.0f, 0.0f}},
		{{+5.0f, +5.0f, -5.0f}, {}, {1.0f, 1.0f}},
		{{+5.0f, +5.0f, +5.0f}, {}, {1.0f, 0.0f}},
		//下
		{{-5.0f, -5.0f, -5.0f}, {}, {0.0f, 1.0f}},
		{{+5.0f, -5.0f, -5.0f}, {}, {0.0f, 0.0f}},
		{{-5.0f, -5.0f, +5.0f}, {}, {1.0f, 1.0f}},
		{{+5.0f, -5.0f, +5.0f}, {}, {1.0f, 0.0f}},
		//上
		{{-5.0f, +5.0f, -5.0f}, {}, {0.0f, 1.0f}},
		{{+5.0f, +5.0f, -5.0f}, {}, {0.0f, 0.0f}},
		{{-5.0f, +5.0f, +5.0f}, {}, {1.0f, 1.0f}},
		{{+5.0f, +5.0f, +5.0f}, {}, {1.0f, 0.0f}},
	};

	///インデックスデータ
	uint16_t indices[] = 
	{
		//前
		0, 1, 2,
		2, 1, 3,
		//後
		5, 4, 6,
		5, 6, 7,
		//左
		8, 9, 10,
		10, 9, 11,
		//右
		13, 12, 14,
		13, 14, 15,
		//下
		16, 17, 18,
		18, 17, 19,
		//上
		21, 20, 22,
		21, 22, 23,
	};

	///法線計算
	for(int i = 0; i < _countof(indices)/3; i++)
	{//三角形一つごとに計算していく
		//三角形にインデックスを取り出して、一時的な変数を入れる
		uint16_t index0 = indices[i*3+0];
		uint16_t index1 = indices[i*3+1];
		uint16_t index2 = indices[i*3+2];
		//三角形を構成する頂点座標をベクトルに代入
		XMVECTOR p0 = XMLoadFloat3(&vertices[index0].pos);
		XMVECTOR p1 = XMLoadFloat3(&vertices[index1].pos);
		XMVECTOR p2 = XMLoadFloat3(&vertices[index2].pos);
		//p0->p1ベクトル、p0->p2ベクトルの計算	(ベクトル減算)
		XMVECTOR v1 = XMVectorSubtract(p1,p0);
		XMVECTOR v2 = XMVectorSubtract(p2,p0);
		//外積は両方から垂直なベクトル
		XMVECTOR normal = XMVector3Cross(v1,v2);
		//正規化(長さを1にする)
		normal = XMVector3Normalize(normal);
		//求めた法線を頂点データに代入
		XMStoreFloat3(&vertices[index0].normal,normal);
		XMStoreFloat3(&vertices[index1].normal,normal);
		XMStoreFloat3(&vertices[index2].normal,normal);
	}


	//頂点データ全体のサイズ = 頂点データ一つ分のサイズ * 頂点データの要素数
	UINT sizeVB = static_cast<UINT>(sizeof(vertices[0]) * _countof(vertices));

	///頂点バッファの確保
	//ヒープ設定
	D3D12_HEAP_PROPERTIES heapProp{};			//ヒープ設定
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;		//GPUへの転送
	//リソース設定
	D3D12_RESOURCE_DESC resDesc{};				//リソース設定
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = sizeVB;			//頂点データ全体のサイズ
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	//生成
	ComPtr<ID3D12Resource> vertBuff;
	result = device->CreateCommittedResource(
		&heapProp,				//ヒープ設定
		D3D12_HEAP_FLAG_NONE,
		&resDesc,				//リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff)
	);
	assert(SUCCEEDED(result));


	///頂点バッファへのデータ転送
	//GPU状のバッファに対応した仮想メモリ(メインメモリ上)を取得
	Vertex* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	assert(SUCCEEDED(result));
	//全頂点に対して
	for(int i = 0; i < _countof(vertices); i++)
	{
		vertMap[i] = vertices[i];	//座標コピー
	}
	//繋がり解除
	vertBuff->Unmap(0, nullptr);


	///頂点バッファビューの作成(GPUに頂点バッファを教えるオブジェクト)
	//作成
	D3D12_VERTEX_BUFFER_VIEW vbView{};
	//GPU仮想アドレス
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	//頂点バッファのサイズ
	vbView.SizeInBytes = sizeVB;
	//頂点一つ分のデータサイズ
	vbView.StrideInBytes = sizeof(vertices[0]);


	///頂点インデックス
	//インデックスデータ全体のサイズ
	UINT sizeIB = static_cast<UINT>(sizeof(uint16_t) * _countof(indices));

	///インデックスバッファの生成
	//リソース設定
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = sizeIB;	//インデックス情報が入る分のサイズ
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//生成
	ComPtr<ID3D12Resource> indexBuff;
	result = device->CreateCommittedResource(
		&heapProp,				//ヒープ設定
		D3D12_HEAP_FLAG_NONE,
		&resDesc,				//リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBuff)
	);
	assert(SUCCEEDED(result));

	///インデックスバッファへのデータ転送
	//マッピング
	uint16_t* indexMap = nullptr;
	result = indexBuff->Map(0, nullptr, (void**)&indexMap);
	assert(SUCCEEDED(result));
	//全インデックスに対して
	for(int i = 0; i < _countof(indices); i++)
	{
		indexMap[i] = indices[i];
	}
	//マッピング解除
	indexBuff->Unmap(0, nullptr);


	///インデックスバッファビューの作成(GPUにインデックスバッファを教えるオブジェクト)
	D3D12_INDEX_BUFFER_VIEW ibView{};
	ibView.BufferLocation = indexBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeIB;



	///頂点シェーダーfileの読み込みとコンパイル
	ComPtr<ID3DBlob> vsBlob ;			//頂点シェーダーオブジェクト
	ComPtr<ID3DBlob> psBlob ;			//ピクセルシェーダーオブジェクト
	ComPtr<ID3DBlob> errorBlob ;		//エラーオブジェクト

	//頂点シェーダーの読み込みコンパイル
	result = D3DCompileFromFile(
		L"BasicVS.hlsl",		//シェーダーファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,	//インクルード可能にする
		"main", "vs_5_0",					//エントリーポイント名、シェーダーモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,	//デバック用設定
		0,
		&vsBlob, &errorBlob);
	//エラーなら
	if(FAILED(result)){
		//errorBlobからエラー内容をstring型にコピー
		std::string error;
		error.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
					errorBlob->GetBufferSize(),
					error.begin());
		error += "\n";
		//エラー内容を出力ウィンドウに表示
		OutputDebugStringA(error.c_str());
		assert(0);
	}

	//ピクセルシェーダーの読み込みコンパイル
	result = D3DCompileFromFile(
		L"BasicPS.hlsl",		//シェーダーファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,	//インクルード可能にする
		"main", "ps_5_0",					//エントリーポイント名、シェーダーモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,	//デバック用設定
		0,
		&psBlob, &errorBlob);
	//エラーなら
	if(FAILED(result)){
		//errorBlobからエラー内容をstring型にコピー
		std::string error;
		error.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
					errorBlob->GetBufferSize(),
					error.begin());
		error += "\n";
		//エラー内容を出力ウィンドウに表示
		OutputDebugStringA(error.c_str());
		assert(0);
	}


	///頂点レイアウト
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
	
		{//xyz座標
			"POSITION",										//セマンティック名
			0,												//同じセマンティック名が複数あるときに使うインデックス
			DXGI_FORMAT_R32G32B32_FLOAT,					//要素数とビット数を表す (XYZの3つでfloat型なのでR32G32B32_FLOAT)
			0,												//入力スロットインデックス
			D3D12_APPEND_ALIGNED_ELEMENT,					//データのオフセット値 (D3D12_APPEND_ALIGNED_ELEMENTだと自動設定)
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,		//入力データ種別 (標準はD3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA)
			0												//一度に描画するインスタンス数
		},
		{//法線ベクトル
			"NORMAL",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
		{//uv座標
			"TEXCOORD",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
	};



	///定数バッファ Color
	//GPUリソースポインタ
	ComPtr<ID3D12Resource> constBufferMaterial ;
	//マッピング用ポインタ
	ConstBufferDataMaterial* constMapMaterial = nullptr;
	{
		//生成用の設定
		//ヒープ設定
		D3D12_HEAP_PROPERTIES cbHeapProp{};
		cbHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;	//GPUへの転送用
		//リソース設定
		D3D12_RESOURCE_DESC cbResourceDesc{};
		cbResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		cbResourceDesc.Width = (sizeof(ConstBufferDataMaterial) + 0xff) & ~0xff;	//256バイトアライメント
		cbResourceDesc.Height = 1;
		cbResourceDesc.DepthOrArraySize = 1;
		cbResourceDesc.MipLevels = 1;
		cbResourceDesc.SampleDesc.Count = 1;
		cbResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		//生成
		result = device->CreateCommittedResource(
			&cbHeapProp,			//ヒープ設定
			D3D12_HEAP_FLAG_NONE,
			&cbResourceDesc,		//リソース設定
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&constBufferMaterial)
		);
		assert(SUCCEEDED(result));

		///定数バッファのマッピング(GPUのVRAMが、CPUのメインメモリに連動)
		result = constBufferMaterial->Map(0, nullptr, (void**)&constMapMaterial);
		assert(SUCCEEDED(result));
	
		///定数バッファへのデータ転送
		//値を書き込むと自動的に転送される
		XMFLOAT4 color = {1.0f, 1.0f, 1.0f, 1.0f};
		constMapMaterial->color = color;	//RGBAで半透明の赤

		//Unmapすると連動が解除される (定数バッファは継続的に値を書き換える用途が多いので、そのままにしてよい)
		//constBufferMaterial->Unmap(0, nullptr);
	}


	//シェーダリソースビューのデスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> srvHeap = nullptr;
	
		///画像ファイルの用意
		TexMetadata metadata{};
		TexMetadata metadata2{};
		ScratchImage scratchImg{};
		ScratchImage scratchImg2{};
		//WICテクスチャデータのロード
		result = LoadFromWICFile(
			L"Resources/Texture.jpg",
			WIC_FLAGS_NONE,
			&metadata, scratchImg);
		assert(SUCCEEDED(result));

		result = LoadFromWICFile(
			L"Resources/Texture2.jpg",
			WIC_FLAGS_NONE,
			&metadata2, scratchImg2);
		assert(SUCCEEDED(result));

		//ミップマップの生成
		ScratchImage mipChain{};
		//生成
		result = GenerateMipMaps(
			scratchImg.GetImages(), 
			scratchImg.GetImageCount(), 
			scratchImg.GetMetadata(),
			TEX_FILTER_DEFAULT, 
			0, 
			mipChain
		);
		if(SUCCEEDED(result))
		{
			scratchImg = std::move(mipChain);
			metadata = scratchImg.GetMetadata();
		}
		ScratchImage mipChain2{};
		//生成
		result = GenerateMipMaps(
			scratchImg2.GetImages(), 
			scratchImg2.GetImageCount(), 
			scratchImg2.GetMetadata(),
			TEX_FILTER_DEFAULT, 
			0, 
			mipChain2
		);
		if(SUCCEEDED(result))
		{
			scratchImg2 = std::move(mipChain2);
			metadata2 = scratchImg2.GetMetadata();
		}

		//フォーマットを書き換える
		//読み込んだディフューズテクスチャをSRGBとして扱う
		metadata.format = MakeSRGB(metadata.format);
		metadata2.format = MakeSRGB(metadata2.format);


		///テクスチャバッファ設定
		//ヒープ設定
		D3D12_HEAP_PROPERTIES textureHandleProp{};
		textureHandleProp.Type = D3D12_HEAP_TYPE_CUSTOM;
		textureHandleProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
		textureHandleProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
		//リソース設定
		D3D12_RESOURCE_DESC textureResourceDesc{};
		textureResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		textureResourceDesc.Format = metadata.format;
		textureResourceDesc.Width = metadata.width;
		textureResourceDesc.Height = (UINT)metadata.height;
		textureResourceDesc.DepthOrArraySize = (UINT16)metadata.arraySize;
		textureResourceDesc.MipLevels = (UINT16)metadata.mipLevels;
		textureResourceDesc.SampleDesc.Count = 1;
		D3D12_RESOURCE_DESC textureResourceDesc2{};
		textureResourceDesc2.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		textureResourceDesc2.Format = metadata2.format;
		textureResourceDesc2.Width = metadata2.width;
		textureResourceDesc2.Height = (UINT)metadata2.height;
		textureResourceDesc2.DepthOrArraySize = (UINT16)metadata2.arraySize;
		textureResourceDesc2.MipLevels = (UINT16)metadata2.mipLevels;
		textureResourceDesc2.SampleDesc.Count = 1;

		//テクスチャバッファの生成
		ComPtr<ID3D12Resource> texBuff ;
		result = device->CreateCommittedResource(
			&textureHandleProp,
			D3D12_HEAP_FLAG_NONE,
			&textureResourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&texBuff)
		);
		assert(SUCCEEDED(result));
		ComPtr<ID3D12Resource> texBuff2 ;
		result = device->CreateCommittedResource(
			&textureHandleProp,
			D3D12_HEAP_FLAG_NONE,
			&textureResourceDesc2,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&texBuff2)
		);
		assert(SUCCEEDED(result));


		//テクスチャバッファへのデータ転送
		//全ミップマップについて
		for(size_t i = 0; i < metadata.mipLevels; i++)
		{
			//ミップマップレベルを指定してイメージを取得
			const Image* img = scratchImg.GetImage(i, 0, 0);
			//テクスチャバッファにデータ転送
			result = texBuff->WriteToSubresource(
				(UINT)i,				
				nullptr,				//全領域へコピー
				img->pixels,			//元データアドレス
				(UINT)img->rowPitch,	//一ラインサイズ
				(UINT)img->slicePitch	//一枚サイズ
			);
			assert(SUCCEEDED(result));
		}
		for(size_t i = 0; i < metadata2.mipLevels; i++)
		{
			//ミップマップレベルを指定してイメージを取得
			const Image* img = scratchImg2.GetImage(i, 0, 0);
			//テクスチャバッファにデータ転送
			result = texBuff2->WriteToSubresource(
				(UINT)i,				
				nullptr,				//全領域へコピー
				img->pixels,			//元データアドレス
				(UINT)img->rowPitch,	//一ラインサイズ
				(UINT)img->slicePitch	//一枚サイズ
			);
			assert(SUCCEEDED(result));
		}



		///デスクリプタヒープ生成
		//SRVの最大個数
		const size_t kMaxSRVCount = 2056;
		//デスクリプタヒープの設定
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;	//シェーダーから見えるように
		srvHeapDesc.NumDescriptors = kMaxSRVCount;
		//設定をもとにSRV用デスクリプタヒープを生成
		result = device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvHeap));
		assert(SUCCEEDED(result));

		///デスクリプタハンドル
		//SRVヒープの先頭ハンドルを取得
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = srvHeap->GetCPUDescriptorHandleForHeapStart();


		///シェーダリソースビューの作成
		//設定
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};	//設定構造体
		srvDesc.Format = textureResourceDesc.Format;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;	//2Dテクスチャ
		srvDesc.Texture2D.MipLevels = textureResourceDesc.MipLevels;

		//ハンドルの指す位置にシェーダーリソースビューの作成
		device->CreateShaderResourceView(texBuff.Get(), &srvDesc, srvHandle);

		//一つハンドルを進める
		UINT incrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		srvHandle.ptr += incrementSize;

		///シェーダリソースビューの作成
		//設定
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};	//設定構造体
		srvDesc2.Format = textureResourceDesc2.Format;
		srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;	//2Dテクスチャ
		srvDesc2.Texture2D.MipLevels = textureResourceDesc2.MipLevels;

		//ハンドルの指す位置にシェーダーリソースビューの作成
		device->CreateShaderResourceView(texBuff2.Get(), &srvDesc2, srvHandle);
	


	//定数バッファ Mat	
	//透視投影
	XMMATRIX matProjection;
	//ビュー変換行列
	XMMATRIX matView;
	XMFLOAT3 eye = {0.0f, 0.0f, -100.0f};	//視点座標
	XMFLOAT3 target= {0, 0, 0};//注視点座標
	XMFLOAT3 up = {0, 1, 0};	//上方向ベクトル

	//3Dオブジェクト数
	const size_t kObjectCount = 50;
	//3dオブジェクトの配列
	Object3d object3ds[kObjectCount];
	
	{
		//設定
		//ヒープ
		D3D12_HEAP_PROPERTIES cbHeapProp{};
		cbHeapProp.Type= D3D12_HEAP_TYPE_UPLOAD;	//GPU転送用
		//リソース
		D3D12_RESOURCE_DESC cbResourceDesc{};
		cbResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		cbResourceDesc.Width = (sizeof(ConstBufferDataTransform)+0xff) & ~0xff;	//256バイトアライメント
		cbResourceDesc.Height = 1;
		cbResourceDesc.DepthOrArraySize = 1;
		cbResourceDesc.MipLevels = 1;
		cbResourceDesc.SampleDesc.Count = 1;
		cbResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		
		//配列内の全オブジェクトに対して
		for(size_t i = 0; i < _countof(object3ds); i++)
		{
			//初期化
			InitializeObject3d(&object3ds[i], device.Get());

			//親子構造体
			//先頭以外
			if(i > 0)
			{
				//一つ前のオブジェクトを親とする
				//object3ds[i].parent = &object3ds[i - 1];

				//Scale
				object3ds[i].scale = {0.9f, 0.9f, 0.9f};
				//rotation
				object3ds[i].rotation = {0.0f, 0.0f, XMConvertToRadians(30.0f)};
				//position
				object3ds[i].position = {0.0f, 0.0f, 8.0f};
			}
		}

	

		//透視投影
		matProjection = XMMatrixPerspectiveFovLH(
			XMConvertToRadians(45.0f),	//上下画角45°
			(float)1280 / 720,			//aspect比(画面横幅/画面縦幅)
			0.1f, 1000.0f				//前端、奥端
		);

		//ビュー変換行列
		matView = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));	
	}



	///ルートパラメータ
	//デスクリプタレンジの設定
	D3D12_DESCRIPTOR_RANGE descriptorRange{};
	descriptorRange.NumDescriptors = 1;			//一度の描画に使うテクスチャが一枚なので1
	descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange.BaseShaderRegister = 0;		//テクスチャレジスタ0番
	descriptorRange.OffsetInDescriptorsFromTableStart =D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	//設定
	D3D12_ROOT_PARAMETER rootParam[3] = {};
	//定数バッファ 0番
	rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;	//定数バッファビュー
	rootParam[0].Descriptor.ShaderRegister = 0;					//定数バッファ番号0
	rootParam[0].Descriptor.RegisterSpace = 0;						//デフォルト値
	rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;	//すべてのシェーダーから見える
	//テクスチャレジスタ 0番
	rootParam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;	//種類
	rootParam[1].DescriptorTable.pDescriptorRanges = &descriptorRange;			//デスクリプタレンジ0
	rootParam[1].DescriptorTable.NumDescriptorRanges = 1;					//デスクリプタレンジ数
	rootParam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;			//全てのシェーダーから見える
	//定数バッファ 1番
	rootParam[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;	//定数バッファビュー
	rootParam[2].Descriptor.ShaderRegister = 1;					//定数バッファ番号1
	rootParam[2].Descriptor.RegisterSpace = 0;					//デフォルト値
	rootParam[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;	//全てのシェーダから見える


	///<summmary>
	///グラフィックスパイプライン
	///<summary/>
	
	//グラフィックスパイプライン設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc{};
	//シェーダー設定
	pipelineDesc.VS.pShaderBytecode = vsBlob->GetBufferPointer();
	pipelineDesc.VS.BytecodeLength  = vsBlob->GetBufferSize();
	pipelineDesc.PS.pShaderBytecode = psBlob->GetBufferPointer();
	pipelineDesc.PS.BytecodeLength  = psBlob->GetBufferSize();
	//サンプルマスク設定
	pipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;	//標準設定
	//ラスタライザ設定
	pipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;	//背面カリング
	pipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;	//ポリゴン内塗りつぶしか
	pipelineDesc.RasterizerState.DepthClipEnable = true;			//深度クリッピングを有効に
	//ブレンドステート
	//レンダーターゲットのブレンド設定
	D3D12_RENDER_TARGET_BLEND_DESC& blenddesc = pipelineDesc.BlendState.RenderTarget[0];
	blenddesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;	//RGBAすべてのチャンネルを描画
	//共通設定
	blenddesc.BlendEnable = true;						//ブレンドを有効にする
	blenddesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;		//加算
	blenddesc.SrcBlendAlpha = D3D12_BLEND_ONE;			//ソースの値を100% 使う	(ソースカラー			 ： 今から描画しようとしている色)
	blenddesc.DestBlendAlpha = D3D12_BLEND_ZERO;		//デストの値を  0% 使う	(デスティネーションカラー： 既にキャンバスに描かれている色)
	//各種設定
	blenddesc.BlendOp = D3D12_BLEND_OP_ADD;	//設定
	blenddesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;			//ソースの値を 何% 使う
	blenddesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;	//デストの値を 何% 使う
	//頂点レイアウト設定
	pipelineDesc.InputLayout.pInputElementDescs = inputLayout;
	pipelineDesc.InputLayout.NumElements = _countof(inputLayout);
	//図形の形状設定 (プリミティブトポロジー)
	pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//その他設定
	pipelineDesc.NumRenderTargets = 1;		//描画対象は一つ
	pipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;	//0~255指定のRGBA
	pipelineDesc.SampleDesc.Count = 1;	//1ピクセルにつき1回サンプリング
	//デプスステンシルステートの設定
	pipelineDesc.DepthStencilState.DepthEnable= true;		//深度テストを行う
	pipelineDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;	//書き込み許可
	pipelineDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;	//小さければ合格
	pipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;	//深度値フォーマット


	///テクスチャサンプラー
	//設定
	D3D12_STATIC_SAMPLER_DESC samplerDesc{};
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;		//横繰り返し(タイリング)
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;		//縦繰り返し(タイリング)
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;		//奥行繰り返し(タイリング)
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;	//ボーダーの時は黒
	samplerDesc.Filter= D3D12_FILTER_MIN_MAG_MIP_LINEAR;		//全てでリニア補間
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;			//ミップマップ最大値
	samplerDesc.MinLOD = 0.0f;						//ミップマップ最小値
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	//ピクセルシェーダからのみ使用可能



	//ルートシグネチャ (テクスチャ、点数バッファなどシェーダーに渡すリソース情報をまとめたオブジェクト)
	//ルートシグネチャの生成
	ComPtr<ID3D12RootSignature> rootSignature;
	//設定
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.pParameters = rootParam;	//ルートパラメータの先頭アドレス
	rootSignatureDesc.NumParameters = _countof(rootParam);		//ルートパラメータ数
	rootSignatureDesc.pStaticSamplers = &samplerDesc;
	rootSignatureDesc.NumStaticSamplers= 1;
	//シリアライズ
	ComPtr<ID3DBlob> rootSigBlob;
	result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
	assert(SUCCEEDED(result));
	result = device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(result));
	//パイプラインにルートシグネチャをセット
	pipelineDesc.pRootSignature = rootSignature.Get();

	//パイプラインステート (グラフィックスパイプラインの設定をまとめたのがパイプラインステートオブジェクト(PSO))
	//パイプラインステートの生成
	ComPtr<ID3D12PipelineState> pipelineState ;
	result = device->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&pipelineState));
	assert(SUCCEEDED(result));



	/// <summary>
	/// DirectX12 描画初期化処理 ここまで
	/// </summary>
	 

	float angle = 0.0f;//カメラの回転角
	bool IsTexture = false;

	//全キーの入力状態を取得する
	const int KeyNum = 256;
	BYTE key[KeyNum] = {};
	BYTE oldkeys[KeyNum] = {};

	/// <summary>
	/// ゲームループ
	/// </summary>
	while(true)
	{
		//メッセージがある?
		if(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);	//キー入力メッセージの処理
			DispatchMessage(&msg);	//プロシージャにメッセージを送る
		}

		//×ボタンで終了メッセージが来たらゲームループを抜ける
		if(msg.message == WM_QUIT)
		{
			break;
		}


		/// <summary>
		/// DirectX12 毎フレーム処理 ここから
		/// </summary>

		///キーボード情報の取得開始
		InputUpdate(keyboard, key, oldkeys, sizeof(key));

		if(IsInKeyTrigger(key, oldkeys, DIK_SPACE))
		{
			if(!IsTexture)
			{
				IsTexture = true;
			}
			else if(IsTexture)
			{
				IsTexture = false;
			}
		}

		if(key[DIK_D] || key[DIK_A])
		{
			if(key[DIK_D])		{angle += XMConvertToRadians(1.0f);}
			else if(key[DIK_A])	{angle -= XMConvertToRadians(1.0f);}

			//angleラジアンだけy軸まわりに回転、半径は-100
			eye.x = -100 * sinf(angle);
			eye.z = -100 * cosf(angle);
			matView = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
		}

		if(key[DIK_UP] || key[DIK_DOWN] || key[DIK_LEFT] || key[DIK_RIGHT])
		{
			if(key[DIK_UP])
			{
				object3ds[0].position.y += 1.0f;
			}
			else if(key[DIK_DOWN])
			{
				object3ds[0].position.y -= 1.0f;
			}

			if(key[DIK_LEFT])
			{
				object3ds[0].position.x -= 1.0f;
			}
			else if(key[DIK_RIGHT])
			{
				object3ds[0].position.x += 1.0f;
			}
		}

		//更新処理
		for(size_t i = 0; i < _countof(object3ds); i++)
		{
			UpdateObject3d(&object3ds[i], matView, matProjection);
		}



		///リソースバリア01
		//バックバッファの番号を取得(ダブルバッファなので 0 or 1)
		UINT bbIndex = swapChain->GetCurrentBackBufferIndex();
		
		//1. リソースバリアで書き込み可能に変更
		D3D12_RESOURCE_BARRIER barrierDesc{};
		barrierDesc.Transition.pResource = backBuffers[bbIndex].Get();					//バックバッファを指定
		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;			//表示状態から
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;	//描画状態へ
		commandList->ResourceBarrier(1, &barrierDesc);


		///描画先指定コマンド
		//2. 描画先の変更
		//レンダーターゲットビューのハンドル取得
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
		rtvHandle.ptr += bbIndex * device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);
		//深度ステンシルビュー用デスクリプタヒープのハンドル取得
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();
		commandList->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);


		///画面クリアコマンド
		//3. 画面クリア
		float clearColor[] = {0.1f, 0.25f, 0.5f, 0.0f};
		//色クリア
		commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		//深度クリア
		commandList->ClearDepthStencilView(dsvHandle,D3D12_CLEAR_FLAG_DEPTH,1.0f, 0, 0, nullptr);


		///描画コマンド
		//4. 描画コマンド ここから

		///<summary>
		///	グラフィックスコマンド
		///<summary/>
		
		///ビューポート
		//設定コマンド
		D3D12_VIEWPORT viewport{};
		viewport.Width = window_width;
		viewport.Height = window_height;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		//ビューポート設定コマンドをコマンドリストに積む
		commandList->RSSetViewports(1, &viewport);
		 
		///シザー矩形
		//設定
		D3D12_RECT scissorRect{};
		scissorRect.left = 0;
		scissorRect.right = scissorRect.left + window_width;
		scissorRect.top = 0;
		scissorRect.bottom = scissorRect.top + window_height;
		//シザー矩形設定コマンドを、コマンドリストに積む
		commandList->RSSetScissorRects(1, &scissorRect);

		///パイプラインステートとルートシグネチャの設定コマンド
		commandList->SetPipelineState(pipelineState.Get());
		commandList->SetGraphicsRootSignature(rootSignature.Get());

		///プリミティブ形状
		//プリミティブ形状の設定コマンド
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		///頂点バッファビュー
		//頂点バッファビューの設定コマンド
		commandList->IASetVertexBuffers(0, 1, &vbView);

		///インデックスバッファビュー
		//インデックスバッファビューの設定コマンド
		commandList->IASetIndexBuffer(&ibView);

		///定数バッファビュー
		//定数バッファビュー(CBV)の設定コマンド	//0番目はCBV
		commandList->SetGraphicsRootConstantBufferView(0, constBufferMaterial->GetGPUVirtualAddress());

		//SRVヒープの設定コマンド	//１番目はSV
		ID3D12DescriptorHeap* ppHeaps[] = {srvHeap.Get()};
		commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		//SRVヒープの先頭ハンドルを取得(SRVをさしているはず)
		D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle = srvHeap->GetGPUDescriptorHandleForHeapStart();
		
		//1枚目SRVヒープの先頭にあるSRVをルートパラメータ1番に設定
		if(!IsTexture)
		{	
			srvGpuHandle = srvHeap->GetGPUDescriptorHandleForHeapStart();
			commandList->SetGraphicsRootDescriptorTable(1, srvGpuHandle);
		}
		//2枚目を指し示すようにしたSRVのハンドルをルートパラメータ1番に設定
		else if(IsTexture)
		{
			srvGpuHandle.ptr += incrementSize;
			commandList->SetGraphicsRootDescriptorTable(1, srvGpuHandle);
		}

		//全オブジェクトについて処理
		for(size_t i = 0; i < _countof(object3ds); i++)
		{
			DrawObject3d(&object3ds[i], commandList.Get(), vbView, ibView, _countof(indices));
		}


		//4. 描画コマンド ここまで


		///リソースバリア02
		//5. リソースバリアを戻す
		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;	//描画状態から
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;			//表示状態へ
		commandList->ResourceBarrier(1, &barrierDesc);


		///コマンドのフラッシュ
		//命令クローズ
		result = commandList->Close();
		assert(SUCCEEDED(result));
		//コマンドリストの実行
		ID3D12CommandList* commandLists[] = {commandList.Get()};
		commandQueue->ExecuteCommandLists(1, commandLists);
		
		//画面に表示するバッファをフリップ(表裏の入れ替え)
		result = swapChain->Present(1, 0);
		assert(SUCCEEDED(result));


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
		result = commandAllocator->Reset();
		assert(SUCCEEDED(result));
		//再びコマンドリストを溜める準備
		result = commandList->Reset(commandAllocator.Get(), nullptr);
		assert(SUCCEEDED(result));


		/// <summary>
		/// DirectX12 毎フレーム処理 ここまで
		/// </summary>

	}
	///ウィンドウクラスを登録解除
	UnregisterClass(w.lpszClassName, w.hInstance);

	return 0;
}

void InputUpdate(IDirectInputDevice8* devkeyboard, BYTE key[], BYTE oldkey[], int arraysize)
{
	devkeyboard->Acquire();
	for(int i = 0; i < arraysize; ++i) oldkey[i] = key[i];

	devkeyboard->GetDeviceState(sizeof(BYTE) * arraysize, key);
}
bool Input(const BYTE key[], int KeysName)
{
	if(key[KeysName])
	{
		return true;
	}
	return false;
}
bool Output(const BYTE key[], int KeysName)
{
	if(!key[KeysName])
	{
		return true;
	}
	return false;
}
bool IsInKeyTrigger(const BYTE key[], const BYTE oldkey[], int KeysName)
{
	if(key[KeysName] && !oldkey[KeysName])
	{
		return true;
	}
	return false;
}
bool IsOutKeyTrigger(const BYTE key[], const BYTE oldkey[], int KeysName)
{
	if(!key[KeysName] && oldkey[KeysName])
	{
		return true;
	}
	return false;
}

void InitializeObject3d(Object3d *object, ID3D12Device* device)
{
	HRESULT result;
	//定数バッファのヒープ設定
	D3D12_HEAP_PROPERTIES heapProp{};
	heapProp.Type =D3D12_HEAP_TYPE_UPLOAD;
	//定数バッファのリソース設定
	D3D12_RESOURCE_DESC resDesc{};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = (sizeof(ConstBufferDataTransform)+ 0xff) & ~0xff;
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//定数バッファの生成
	result = device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&object->constBuffTransform));
	assert(SUCCEEDED(result));

	//定数バッファのマッピング
	result = object->constBuffTransform->Map(0,nullptr, (void**)&object->constMapTransform);
	assert(SUCCEEDED(result));
}

void UpdateObject3d(Object3d *object, XMMATRIX &matView, XMMATRIX &matProjection)
{
	XMMATRIX matScale, matRot, matTrans;

	//スケール、回転、平行移動行列の計算
	matScale = XMMatrixScaling(object->scale.x, object->scale.y, object->scale.z);
	matRot = XMMatrixIdentity();
	matRot *= XMMatrixRotationZ(object->rotation.z);
	matRot *= XMMatrixRotationX(object->rotation.x);
	matRot *= XMMatrixRotationY(object->rotation.y);
	matTrans = XMMatrixTranslation(object->position.x, object->position.y, object->position.z);

	//ワールド行列の合成
	object->matWorld = XMMatrixIdentity();	//変形をリセット
	object->matWorld *= matScale;			//ワールド行列にスケーリングを反映
	object->matWorld *= matRot;				//ワールド行列に回転を反映
	object->matWorld *= matTrans;			//ワールド行列に平行移動を反映

	//親オブジェクトの存在
	if(object->parent != nullptr)
	{
		//親オブジェクトのワールド行列を掛ける
		object->matWorld *= object->parent->matWorld;
	}

	//定数バッファへのデータ転送
	object->constMapTransform->mat = object->matWorld * matView *matProjection;
}

void DrawObject3d(Object3d *object, ID3D12GraphicsCommandList* commandList, D3D12_VERTEX_BUFFER_VIEW &vbView, D3D12_INDEX_BUFFER_VIEW &ibView, UINT numIndices)
{
	//頂点バッファの設定
	commandList->IASetVertexBuffers(0, 1, &vbView);
	//インデックスバッファの設定
	commandList->IASetIndexBuffer(&ibView);
	//定数バッファビュー(CBVの設定コマンド)
	commandList->SetGraphicsRootConstantBufferView(2, object->constBuffTransform->GetGPUVirtualAddress());
	//描画コマンド
	commandList->DrawIndexedInstanced(numIndices,1, 0, 0, 0);
}
