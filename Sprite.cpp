﻿#include "Sprite.h"
#include <d3dcompiler.h>
#include <d3dx12.h>
#include "TextureManager.h"

#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;
using namespace Microsoft::WRL;

/// <summary>
/// 静的メンバ変数の実体
/// </summary>
ID3D12Device* Sprite::sDevice_ = nullptr;
UINT Sprite::sDescriptorHandleIncrementSize_;
ID3D12GraphicsCommandList* Sprite::sCommandList_ = nullptr;
ComPtr<ID3D12RootSignature> Sprite::sRootSignature_;
ComPtr<ID3D12PipelineState> Sprite::sPipelineStates_;
XMMATRIX Sprite::sMatProjection_;

void Sprite::StaticInitialize(ID3D12Device *device, int window_width, int window_height, const std::wstring &directoryPath)
{
	sDevice_ = device;
	sDescriptorHandleIncrementSize_ = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	InitializeGraphicsPipeline();

	//射影変換
	sMatProjection_ = XMMatrixOrthographicOffCenterLH
	(
		0.f, (float)window_width,
		(float)window_height, 0.f,
		0.f, 1.f
	);
}

void Sprite::PreDraw(ID3D12GraphicsCommandList *commandList)
{
	assert(Sprite::sCommandList_ == nullptr);

	sCommandList_ = commandList;

	//パイプラインステートの設定
	sCommandList_->SetPipelineState(sPipelineStates_.Get());
	//ルートシグネチャの設定
	sCommandList_->SetGraphicsRootSignature(sRootSignature_.Get());
	//プリミティブ形状を設定
	sCommandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

void Sprite::PostDraw()
{
	//コマンドリスト解除
	sCommandList_ = nullptr;
}

void Sprite::InitializeGraphicsPipeline()
{
	HRESULT result;

	///頂点シェーダーfileの読み込みとコンパイル
	ComPtr<ID3DBlob> vsBlob ;			//頂点シェーダーオブジェクト
	ComPtr<ID3DBlob> psBlob ;			//ピクセルシェーダーオブジェクト
	ComPtr<ID3DBlob> errorBlob ;		//エラーオブジェクト

	//頂点シェーダーの読み込みコンパイル
	result = D3DCompileFromFile(
		L"SpriteVS.hlsl",		//シェーダーファイル名
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
		L"SpritePS.hlsl",		//シェーダーファイル名
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

	///ルートパラメータ
	//デスクリプタレンジの設定
	CD3DX12_DESCRIPTOR_RANGE descRangeSRV{};
	descRangeSRV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);


	//設定
	////定数バッファ 0番
	CD3DX12_ROOT_PARAMETER rootParam[2] = {};
	////定数　0番 material
	rootParam[0].InitAsConstantBufferView(0);
	////テクスチャレジスタ 0番
	rootParam[1].InitAsDescriptorTable(1, &descRangeSRV);


	///<summmary>
	///グラフィックスパイプライン
	///<summary/>
	
	//グラフィックスパイプライン設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc{};
	//シェーダー設定
	pipelineDesc.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
	pipelineDesc.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());
	
	//サンプルマスク設定
	pipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;	//標準設定
	//ラスタライザ設定 背面カリング	ポリゴン内塗りつぶし	深度クリッピング有効
	pipelineDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	pipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
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
	//デプスステンシルステートの設定	(深度テストを行う、書き込み許可、深度がちいさければ許可)
	pipelineDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	pipelineDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pipelineDesc.DepthStencilState.DepthEnable = false;
	pipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;	//深度値フォーマット

	///テクスチャサンプラー
	//設定
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0);


	//ルートシグネチャ (テクスチャ、定数バッファなどシェーダーに渡すリソース情報をまとめたオブジェクト)
	//設定
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Init_1_0(_countof(rootParam), rootParam,1, &samplerDesc,D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	//シリアライズ
	ComPtr<ID3DBlob> rootSigBlob;
	result = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob,&errorBlob);
	assert(SUCCEEDED(result));
	result = sDevice_->CreateRootSignature(0,rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(),IID_PPV_ARGS(&sRootSignature_));
	assert(SUCCEEDED(result));
	//パイプラインにルートシグネチャをセット
	pipelineDesc.pRootSignature = sRootSignature_.Get();

	//パイプラインステート (グラフィックスパイプラインの設定をまとめたのがパイプラインステートオブジェクト(PSO))
	//パイプラインステートの生成
	result = sDevice_->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&sPipelineStates_));
	assert(SUCCEEDED(result));
}

Sprite *Sprite::Create(uint32_t textureHandle, DirectX::XMFLOAT2 position, DirectX::XMFLOAT4 color, DirectX::XMFLOAT2 anchorpoint, bool isFlipX, bool isFlipY)
{
	XMFLOAT2 size = {100.f, 100.f};

	{
		//テクスチャ情報
		const D3D12_RESOURCE_DESC& resDesc = TextureManager::GetInstance()->GetResourceDesc(textureHandle);
		//スプライトのサイズをテクスチャのサイズ
		size = {(float)resDesc.Width, (float)resDesc.Height};
	}

	//インスタンス生成
	Sprite* sprite = new Sprite(textureHandle, position, size, color, anchorpoint, isFlipX, isFlipY);
	if(sprite == nullptr)
	{
		return nullptr;
	}

	//初期化
	if(!sprite->Initialize())
	{
		delete sprite;
		assert(0);
		return nullptr;
	}

	return sprite;
}

Sprite::Sprite(uint32_t textureHandle, DirectX::XMFLOAT2 position, DirectX::XMFLOAT2 size, DirectX::XMFLOAT4 color, DirectX::XMFLOAT2 anchorPoint, bool isFlipX, bool isFlipY)
{
	this->textureHandle = textureHandle;
	this->position = position;
	this->size = size;
	this->texSize = size;
	this->color = color;
	this->anchorPoint = anchorPoint;
	this->isFlipX = isFlipX;
	this->isFlipY = isFlipY;
	this->matWorld = XMMatrixIdentity();
}

bool Sprite::Initialize()
{
	HRESULT result = S_FALSE;

	resourceDesc = TextureManager::GetInstance()->GetResourceDesc(textureHandle);

	{
		//ヒーププロパティ
		CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		//リソース設定
		CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(VertexPosUv) * kVertNum);

		//頂点バッファ生成
		result =sDevice_->CreateCommittedResource
			(
				&heapProp, D3D12_HEAP_FLAG_NONE,
				&resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr, IID_PPV_ARGS(&vertBuff)
			);
		assert(SUCCEEDED(result));

		//頂点バッファマッピング
		result = vertBuff->Map(0, nullptr, (void**)&vertMap);
		assert(SUCCEEDED(result));
	}

	//頂点バッファへのデータ転送
	TransferVertices();

	//頂点バッファへのデータ転送
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	vbView.SizeInBytes = sizeof(VertexPosUv) * 4;
	vbView.StrideInBytes = sizeof(VertexPosUv);

	{
		//ヒーププロパティ
		CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		//リソース設定
		CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferData) + 0xff) & ~0xff);

		//定数バッファ生成
		result = sDevice_->CreateCommittedResource
			(
				&heapProp, D3D12_HEAP_FLAG_NONE,
				&resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr, IID_PPV_ARGS(&constBuff)
			);
		assert(SUCCEEDED(result));
	}

	//定数バッファマッピング
	result = constBuff->Map(0, nullptr, (void**)&constMap);
	assert(SUCCEEDED(result));

	return true;
}

void Sprite::TransferVertices()
{
	HRESULT result = S_FALSE;

	// 左下、左上、右下、右上
	enum { LB, LT, RB, RT };

	float left = (0.0f - anchorPoint.x) * size.x;
	float right = (1.0f - anchorPoint.x) * size.x;
	float top = (0.0f - anchorPoint.y) * size.y;
	float bottom = (1.0f - anchorPoint.y) * size.y;
	if (isFlipX) { // 左右入れ替え
		left = -left;
		right = -right;
	}

	if (isFlipY) { // 上下入れ替え
		top = -top;
		bottom = -bottom;
	}

	// 頂点データ
	VertexPosUv vertices[4];

	vertices[LB].pos = {left, bottom, 0.0f};  // 左下
	vertices[LT].pos = {left, top, 0.0f};     // 左上
	vertices[RB].pos = {right, bottom, 0.0f}; // 右下
	vertices[RT].pos = {right, top, 0.0f};    // 右上

	// テクスチャ情報取得
	{
		float tex_left = texBase.x / resourceDesc.Width;
		float tex_right = (texBase.x + texSize.x) / resourceDesc.Width;
		float tex_top = texBase.y / resourceDesc.Height;
		float tex_bottom = (texBase.y + texSize.y) / resourceDesc.Height;

		vertices[LB].uv = {tex_left, tex_bottom};  // 左下
		vertices[LT].uv = {tex_left, tex_top};     // 左上
		vertices[RB].uv = {tex_right, tex_bottom}; // 右下
		vertices[RT].uv = {tex_right, tex_top};    // 右上
	}

	// 頂点バッファへのデータ転送
	memcpy(vertMap, vertices, sizeof(vertices));
}

void Sprite::Draw()
{
	//ワールド行列の更新
	this->matWorld = XMMatrixIdentity();
	//Z軸回転
	this->matWorld *= XMMatrixRotationZ(XMConvertToRadians(this->rotation));
	//平行移動
	this->matWorld *= XMMatrixTranslation(this->position.x, this->position.y, 0.f);

	//定数バッファ転送
	constMap->mat = this->matWorld * sMatProjection_;
	constMap->color = this->color;

	//頂点バッファのセット
	sCommandList_->IASetVertexBuffers(0,1,&this->vbView);
	//定数バッファをセット
	sCommandList_->SetGraphicsRootConstantBufferView(0, this->constBuff->GetGPUVirtualAddress());
	//シェーダリソースビュー設定
	TextureManager::GetInstance()->SetGraphicsRootDescriptorTable(sCommandList_, 1, textureHandle);
	//ポリゴンの描画
	sCommandList_->DrawInstanced(4, 1, 0, 0);
}

void Sprite::SetTextureHandle(const uint32_t textureHandle)
{
	this->textureHandle = textureHandle;
	resourceDesc = TextureManager::GetInstance()->GetResourceDesc(this->textureHandle);
}

void Sprite::SetPosition(const DirectX::XMFLOAT2 &position)
{
	this->position = position;

	TransferVertices();
}

void Sprite::SetRotation(const float rotation)
{
	this->rotation = rotation;

	TransferVertices();
}

void Sprite::SetSize(const DirectX::XMFLOAT2 &size)
{
	this->size = size;

	TransferVertices();
}

void Sprite::SetAnchorPoint(const DirectX::XMFLOAT2 &anchorpoint)
{
	this->anchorPoint = anchorPoint;

	TransferVertices();
}

void Sprite::SetIsFlipX(const bool isFlipX)
{
	this->isFlipX = isFlipX;

	TransferVertices();
}

void Sprite::SetIsFlipY(bool isFlipY)
{
	this->isFlipY = isFlipY;

	TransferVertices();
}

void Sprite::SetTextureRect(const DirectX::XMFLOAT2 &texBase, const DirectX::XMFLOAT2 &texSize)
{
	this->texBase = texBase;
	this->texSize = texSize;

	TransferVertices();
}

