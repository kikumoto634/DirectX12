#include "FbxModelMesh.h"


void FbxModelMesh::CreateBuffers()
{
	HRESULT result = S_FALSE;

	//���_�f�[�^�̑S�̃T�C�Y
	UINT sizeVB = static_cast<UINT>(sizeof(VertexPosNormalUVSkin)*vertices.size());

	//�q�[�v�v���p�e�B
	CD3DX12_HEAP_PROPERTIES heapProp= CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	//���\�[�X�ݒ�
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeVB);

	//���_�o�b�t�@
}

void FbxModelMesh::Draw(ID3D12GraphicsCommandList *commandList)
{
}
