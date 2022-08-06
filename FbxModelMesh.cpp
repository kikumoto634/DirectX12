#include "FbxModelMesh.h"


void FbxModelMesh::CreateBuffers()
{
	HRESULT result = S_FALSE;

	//頂点データの全体サイズ
	UINT sizeVB = static_cast<UINT>(sizeof(VertexPosNormalUVSkin)*vertices.size());

	//ヒーププロパティ
	CD3DX12_HEAP_PROPERTIES heapProp= CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	//リソース設定
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeVB);

	//頂点バッファ
}

void FbxModelMesh::Draw(ID3D12GraphicsCommandList *commandList)
{
}
