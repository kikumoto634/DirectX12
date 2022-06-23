#include "TextureManager.h"

#include <cassert>

using namespace DirectX;

void TextureManager::Inithalize(DirectXCommon* dxCommon)
{
	HRESULT result;
	//メンバ変数に記録
	this->dxCommon = dxCommon;

	//デスクリプタヒープの設定
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;	//シェーダーから見えるように
	srvHeapDesc.NumDescriptors = spriteSRVCount;
	//設定をもとにSRV用デスクリプタヒープを生成
	result = this->dxCommon->GetDevice()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&spriteDescHeap));
	if(FAILED(result))
	{
		assert(0);
	}
}

void TextureManager::LoadTexture(UINT texnumber, const wchar_t *filename)
{
	HRESULT result;

	///画像ファイルの用意
	TexMetadata metadata{};
	ScratchImage scratchImg{};
	//WICテクスチャデータのロード
	result = LoadFromWICFile(
		filename,
		WIC_FLAGS_NONE,
		&metadata, scratchImg);
	if(FAILED(result))
	{
		assert(0);
	}

	const Image* img = scratchImg.GetImage(0, 0, 0);

	//リソース設定
	CD3DX12_RESOURCE_DESC texresDesc = CD3DX12_RESOURCE_DESC::Tex2D
	(
		metadata.format,
		metadata.width,
		(UINT)metadata.height,
		(UINT16)metadata.arraySize,
		(UINT16)metadata.mipLevels
	);

	//テクスチャバッファの生成
	result= dxCommon->GetDevice()->CreateCommittedResource
	(
		&CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0),
		D3D12_HEAP_FLAG_NONE,
		&texresDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&spriteTexBuff[texnumber])
	);
	if(FAILED(result))
	{
		assert(0);
	}


	//テクスチャバッファにデータ転送
	result = spriteTexBuff[texnumber]->WriteToSubresource(
		0,				
		nullptr,				//全領域へコピー
		img->pixels,			//元データアドレス
		(UINT)img->rowPitch,	//一ラインサイズ
		(UINT)img->slicePitch	//一枚サイズ
	);
	if(FAILED(result))
	{
		assert(0);
	}

	///シェーダリソースビューの作成
	//設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};	//設定構造体
	D3D12_RESOURCE_DESC resDesc = spriteTexBuff[texnumber]->GetDesc();


	srvDesc.Format = resDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;	//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = 1;

	//ハンドルの指す位置にシェーダーリソースビューの作成
	dxCommon->GetDevice()->CreateShaderResourceView
	(
		spriteTexBuff[texnumber].Get(),
		&srvDesc,
		CD3DX12_CPU_DESCRIPTOR_HANDLE//SRVヒープの先頭ハンドルを取得
			(
				spriteDescHeap->GetCPUDescriptorHandleForHeapStart(),
				0,
				dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
			)
	);
}

ID3D12Resource *TextureManager::GetSpriteTexBuffer(UINT texnumber)
{
	//配列オーバーフロー防止
	assert(texnumber < spriteSRVCount);

	return spriteTexBuff[texnumber].Get();
}

void TextureManager::SetDescriptorHeaps(ID3D12GraphicsCommandList *commandList)
{
	ID3D12DescriptorHeap* ppHeaps[] = {spriteDescHeap.Get()};
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
}

void TextureManager::SetShaderResourceView(ID3D12GraphicsCommandList *commandList, UINT rootParameterIndex, UINT texnumber)
{
	///SRVのサイズを取得
	UINT sizeSRV = dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//デスクリプタテーブルの先頭を取得
	D3D12_GPU_DESCRIPTOR_HANDLE start = spriteDescHeap->GetGPUDescriptorHandleForHeapStart();

	//SRVのGPUハンドル取得
	CD3DX12_GPU_DESCRIPTOR_HANDLE handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(start, texnumber, sizeSRV);

	//SRVをセット
	dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(rootParameterIndex, handle);
}
