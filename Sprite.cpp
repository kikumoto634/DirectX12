#include "Sprite.h"
#include <cassert>
#include <d3dcompiler.h>
#include <d3d12.h>
#include "TextureManager.h"

#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;

void Sprite::StaticInitialize(DirectXCommon* dxCommon, TextureManager* texManager)
{
	common = new Common();

	common->dxCommon = dxCommon;
	common->textureManager = texManager;

	//グラフィックスパイプライン生成
	common->InitializeGraphicsPipeline();

	////並行投影の射影行列生成
	//common->matProjection = XMMatrixOrthographicOffCenterLH
	//(
	//	0.f, WinApp::window_width,
	//	WinApp::window_height, 0.f,
	//	0.f, 1.f
	//);
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
	this->matWorld *= XMMatrixTranslation(this->position.m128_f32[0], this->position.m128_f32[1], this->position.m128_f32[2]);

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


void Sprite::StaticInitialize(ID3D12Device *device, int window_width, int window_height, const std::wstring &directoryPath)
{
	Sprite::device = device;

	Sprite::descriptorHandleSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void Sprite::PreDraw(ID3D12GraphicsCommandList *commandList)
{
}

void Sprite::PostDraw()
{
}

Sprite *Sprite::Create(uint32_t textureHandle, XMFLOAT2 pos, XMFLOAT4 color, XMFLOAT2 anchor, bool isFlipX, bool isFlipY)
{
	return nullptr;
}


Sprite::Sprite(uint32_t textureHandle, XMFLOAT2 pos, XMFLOAT2 size, XMFLOAT4 color, XMFLOAT2 anchor, bool isFlipX, bool isFlipY)
{
}

bool Sprite::Initialize()
{
	return false;
}

void Sprite::SetTextureHandle(uint32_t textureHandle)
{
}

void Sprite::SetRotation(float rotation)
{
}

void Sprite::SetSize(const XMFLOAT2 &size)
{
}

void Sprite::SetAnchorPoint(const XMFLOAT2 &anchorpoint)
{
}

void Sprite::SetIsFlipX(bool IsFlipX)
{

}

void Sprite::SetIsFlipY(bool IsFlipY)
{

}

void Sprite::SetTextureRect(const XMFLOAT2 &texBase, const XMFLOAT2 &texSize)
{
}

void Sprite::Draw()
{
}

void Sprite::TransferVertices()
{
}

