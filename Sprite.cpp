﻿#include "Sprite.h"

#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;

Sprite::Common* Sprite::common = nullptr;

void Sprite::StaticInitialize(DirectXCommon* dxCommon, TextureManager* texManager)
{
	common = new Common();

	common->dxCommon = dxCommon;
	common->textureManager = texManager;

	//グラフィックスパイプライン生成
	common->InitializeGraphicsPipeline();

	//並行投影の射影行列生成
	common->matProjection = XMMatrixOrthographicOffCenterLH
	(
		0.f, WinApp::window_width,
		WinApp::window_height, 0.f,
		0.f, 1.f
	);
}

void Sprite::StaticFinalize()
{
	//解放
	if(common != nullptr)
	{
		delete common;
		common = nullptr;
	}
}

void Sprite::SetPipelineState(ID3D12GraphicsCommandList *commandList)
{
	//パイプラインステートの設定
	commandList->SetPipelineState(common->pipelinestate.Get());
	//ルートシグネチャの設定
	commandList->SetGraphicsRootSignature(common->rootsignature.Get());
	//プリミティブ形状を設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

void Sprite::Initialize(UINT texNumber)
{
	HRESULT result;

	//テクスチャ番号コピー
	this->texNumber = texNumber;

	

	//指定番号の画像が読込落ちなら
	if(common->textureManager->GetSpriteTexBuffer(this->texNumber))
	{
		//テクスチャ情報取得
		D3D12_RESOURCE_DESC resDesc = common->textureManager->GetSpriteTexBuffer(this->texNumber)->GetDesc();

		//スプライトの大きさを画像の解像度に合わせる
		this->size = {(float)resDesc.Width, (float)resDesc.Height};
	}

	Sprite sprite{};

	VertexPosUv vertices[] = 
	{
		{{  0.f, 100.f, 0.f}, {0.f, 1.f}},
		{{  0.f,   0.f, 0.f}, {0.f, 0.f}},
		{{100.f, 100.f, 0.f}, {1.f, 1.f}},
		{{100.f,   0.f, 0.f}, {1.f, 0.f}},
	};

	//頂点バッファ生成
	result = common->dxCommon->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&this->vertBuff)
	);

	//頂点バッファへのデータ転送
	SpriteTransferVertexBuffer();

	//頂点バッファビューの作成
	this->vbView.BufferLocation = this->vertBuff->GetGPUVirtualAddress();
	this->vbView.SizeInBytes = sizeof(vertices);
	this->vbView.StrideInBytes = sizeof(vertices[0]);

	//定数バッファの生成
	result = common->dxCommon->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferData) + 0xff)&~0xff),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&this->constBuffData)
	);

	//定数バッファのデータ転送
	ConstBufferData* constMap = nullptr;
	result = this->constBuffData->Map(0, nullptr, (void**)&constMap);
	if(SUCCEEDED(result)){
		constMap->color = XMFLOAT4(1.f, 1.f, 1.f, 1.f);
		constMap->mat = common->matProjection;
		this->constBuffData->Unmap(0, nullptr);
	}
}

//スプライト単体頂点バッファの転送
void Sprite::SpriteTransferVertexBuffer()
{
	HRESULT result = S_FALSE;

	//頂点データ
	VertexPosUv vertices[] = 
	{
		{{}, {0.f, 1.f}},
		{{}, {0.f, 0.f}},
		{{}, {1.f, 1.f}},
		{{}, {1.f, 0.f}},
	};

	enum {LB, LT, RB, RT};

	float left = (0.f - this->anchorpoint.x)* this->size.x;
	float right = (1.f - this->anchorpoint.x)* this->size.x;
	float top = (0.f - this->anchorpoint.y)* this->size.y;
	float bottom = (1.f - this->anchorpoint.y)* this->size.y;

	if(this->IsFlipX)
	{//左右入れ替え
		left = -left;
		right = -right;
	}
	if(this->IsFlipY)
	{//上下反転
		top = -top;
		bottom = -bottom;
	}

	vertices[LB].pos = {left, bottom, 0.f};
	vertices[LT].pos = {left, top, 0.f};
	vertices[RB].pos = {right, bottom, 0.f};
	vertices[RT].pos = {right, top, 0.f};

	//UV計算
	//指定番号の画像が読込済みなら
	if(common->textureManager->GetSpriteTexBuffer(this->texNumber))
	{
		//テクスチャ情報取得
		D3D12_RESOURCE_DESC resDesc = common->textureManager->GetSpriteTexBuffer(this->texNumber)->GetDesc();

		float tex_left = this->texLeftTop.x / resDesc.Width;
		float tex_right = (this->texLeftTop.x + this->texSize.x) / resDesc.Width;
		float tex_top = this->texLeftTop.y / resDesc.Height;
		float tex_bottom = (this->texLeftTop.x + this->texSize.y) / resDesc.Height;
	
		vertices[LB].uv = {tex_left, tex_bottom};
		vertices[LT].uv = {tex_left, tex_top};
		vertices[RB].uv = {tex_right, tex_bottom};
		vertices[RT].uv = {tex_right, tex_top};
	}

	//頂点バッファのデータ転送
	VertexPosUv* vertMap = nullptr;
	result = this->vertBuff->Map(0, nullptr, (void**)&vertMap);
	memcpy(vertMap, vertices,sizeof(vertices));
	this->vertBuff->Unmap(0,nullptr);
}

void Sprite::Draw(ID3D12GraphicsCommandList *commandList)
{
	//ワールド行列の更新
	this->matWorld = XMMatrixIdentity();

	//Z軸回転
	this->matWorld *= XMMatrixRotationZ(XMConvertToRadians(this->rotation));

	//平行移動
	this->matWorld *= XMMatrixTranslation(this->position.x, this->position.y, this->position.z);

	//定数バッファの転送
	ConstBufferData* constMap = nullptr;
	HRESULT result = this->constBuffData->Map(0,nullptr, (void**)&constMap);
	constMap->mat = this->matWorld * common->matProjection;
	constMap->color = this->color;
	this->constBuffData->Unmap(0, nullptr);

	if(this->IsInvisible)
	{
		return ;
	}

	//頂点バッファのセット
	commandList->IASetVertexBuffers(0,1,&this->vbView);
	//定数バッファをセット
	commandList->SetGraphicsRootConstantBufferView(0, this->constBuffData->GetGPUVirtualAddress());
	//テスクチャ用デスクリプタヒープの設定
	common->textureManager->SetDescriptorHeaps(commandList);
	//シェーダーリソースビューをセット
	common->textureManager->SetShaderResourceView(commandList, 1, this->texNumber);
	//ポリゴンの描画
	commandList->DrawInstanced(4, 1, 0, 0);
}

Vector2 Sprite::GetPosition()
{
	return Vector2(position.x, position.y);
}

void Sprite::SetPosition(Vector2 pos)
{
	this->position.x = pos.x;
	this->position.y = pos.y;

	//頂点情報の転送
	SpriteTransferVertexBuffer();
}

Vector2 Sprite::GetSize()
{
	return Vector2(size.x, size.y);
}

void Sprite::SetSize(Vector2 size)
{
	this->size.x = size.x;
	this->size.y = size.y;
	
	//頂点情報の転送
	SpriteTransferVertexBuffer();
}

void Sprite::SetAnchorpoint(Vector2 pos)
{
	this->anchorpoint.x = pos.x;
	this->anchorpoint.y = pos.y;

	SpriteTransferVertexBuffer();
}

void Sprite::SetTextureRect(float tex_x, float tex_y, float tex_width, float tex_height)
{
	this->texLeftTop = {tex_x, tex_y};
	this->texSize = {tex_width, tex_height};

	SpriteTransferVertexBuffer();
}

void Sprite::SetIsFlipX(bool IsFlipX)
{
	this->IsFlipX = IsFlipX;

	SpriteTransferVertexBuffer();
}

void Sprite::SetIsFlipY(bool IsFlipY)
{
	this->IsFlipY = IsFlipY;

	SpriteTransferVertexBuffer();
}

void Sprite::Common::InitializeGraphicsPipeline()
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
	if(FAILED(result))
	{
		assert(0);
	}
	result = dxCommon->GetDevice()->CreateRootSignature(0,rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(),IID_PPV_ARGS(&common->rootsignature));
	if(FAILED(result))
	{
		assert(0);
	}
	//パイプラインにルートシグネチャをセット
	pipelineDesc.pRootSignature = common->rootsignature.Get();

	//パイプラインステート (グラフィックスパイプラインの設定をまとめたのがパイプラインステートオブジェクト(PSO))
	//パイプラインステートの生成
	result = dxCommon->GetDevice()->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&common->pipelinestate));
	if(FAILED(result))
	{
		assert(0);
	}
}
