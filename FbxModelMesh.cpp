#include "FbxModelMesh.h"
#include "DirectXCommon.h"
#include <cassert>

using namespace std;
using namespace DirectX;

void FbxModelMesh::CreateBuffers()
{
	HRESULT result = S_FALSE;

	//頂点データの全体サイズ
	UINT sizeVB = static_cast<UINT>(sizeof(VertexPosNormalUVSkin)*vertices.size());

	//ヒーププロパティ
	CD3DX12_HEAP_PROPERTIES heapProp= CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	//リソース設定
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeVB);
	//頂点バッファ生成
	result = DirectXCommon::GetInstance()->GetDevice()->CreateCommittedResource
		(
			&heapProp, D3D12_HEAP_FLAG_NONE,
			&resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, IID_PPV_ARGS(&vertBuff)
		);
	assert(SUCCEEDED(result));

	//頂点バッファ転送
	VertexPosNormalUVSkin* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	if(SUCCEEDED(result))
	{
		copy(vertices.begin(), vertices.end(), vertMap);
		vertBuff->Unmap(0, nullptr);
	}
	//頂点バッファビュー生成
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	vbView.SizeInBytes = sizeVB;
	vbView.StrideInBytes = sizeof(vertices[0]);


	//頂点インデックス全体のサイズ
	UINT sizeIB = static_cast<UINT>(sizeof(unsigned short) * indices.size());
	//リソース
	resourceDesc.Width = sizeIB;
	//インデックスバッファ生成
	result = DirectXCommon::GetInstance()->GetDevice()->CreateCommittedResource
		(
			&heapProp, D3D12_HEAP_FLAG_NONE,
			&resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, IID_PPV_ARGS(&indexBuff)
		);
	assert(SUCCEEDED(result));

	//インデックスバッファ転送
	unsigned short* indexMap = nullptr;
	result = indexBuff->Map(0, nullptr,(void**)&indexMap);
	if(SUCCEEDED(result))
	{
		copy(indices.begin(),indices.end(),indexMap);
		indexBuff->Unmap(0, nullptr);
	}
	//インデックスバッファビュー(IBV)の作成
	ibView.BufferLocation = indexBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes= sizeIB;

	//テクスチャデータ
	const Image* img = scratchImg.GetImage(0,0,0);
	assert(img);
	//リソース
	CD3DX12_RESOURCE_DESC texResDesc = CD3DX12_RESOURCE_DESC::Tex2D
		(
			metaData.format,
			metaData.width,
			(UINT)metaData.height,
			(UINT)metaData.arraySize,
			(UINT)metaData.mipLevels
		);
	//ヒーププロパティ
	CD3DX12_HEAP_PROPERTIES texHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0);
	//テクスチャバッファ生成
	result = DirectXCommon::GetInstance()->GetDevice()->CreateCommittedResource
		(
			&texHeapProp, D3D12_HEAP_FLAG_NONE,
			&texResDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, IID_PPV_ARGS(&texBuff)
		);
	assert(SUCCEEDED(result));

	//テクスチャバッファ転送
	result = texBuff->WriteToSubresource
		(
			0, nullptr,img->pixels,
			(UINT)img->rowPitch, (UINT)img->slicePitch
		);
	assert(SUCCEEDED(result));


	//SRVデスクリプタヒープ生成
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NumDescriptors = 1;
	result = DirectXCommon::GetInstance()->GetDevice()->CreateDescriptorHeap
		(
			&descHeapDesc, IID_PPV_ARGS(&descHeapsSRV)
		);
	assert(SUCCEEDED(result));

	//シェーダリソースビュー
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};	//設定構造体
	D3D12_RESOURCE_DESC resDesc= texBuff->GetDesc();

	srvDesc.Format = resDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension =D3D12_SRV_DIMENSION_TEXTURE2D;	//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = 1;

	DirectXCommon::GetInstance()->GetDevice()->CreateShaderResourceView
	(
		texBuff.Get(),	//ビューと関連付けるバッファ
		&srvDesc,	//テクスチャ設定情報
		descHeapsSRV->GetCPUDescriptorHandleForHeapStart()	//ヒープの先頭アドレス
	);
}

void FbxModelMesh::Draw(ID3D12GraphicsCommandList *commandList, UINT rootParameterIndex)
{
	//頂点バッファをセット
	commandList->IASetVertexBuffers(0, 1, &vbView);
	//インデックスバッファをセット
	commandList->IASetIndexBuffer(&ibView);

	//デスクリプタヒープのセット
	ID3D12DescriptorHeap* ppHeaps[] = {descHeapsSRV.Get()};
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	//シェーダリソースビューをセット
	commandList->SetGraphicsRootDescriptorTable(rootParameterIndex, descHeapsSRV->GetGPUDescriptorHandleForHeapStart());

	//描画コマンド
	commandList->DrawIndexedInstanced((UINT)indices.size(), 1, 0, 0, 0);
}
