#include "FbxModelMesh.h"
#include "DirectXCommon.h"
#include <cassert>

using namespace std;
using namespace DirectX;

void FbxModelMesh::CreateBuffers()
{
	HRESULT result = S_FALSE;

	//���_�f�[�^�̑S�̃T�C�Y
	UINT sizeVB = static_cast<UINT>(sizeof(VertexPosNormalUVSkin)*vertices.size());

	//�q�[�v�v���p�e�B
	CD3DX12_HEAP_PROPERTIES heapProp= CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	//���\�[�X�ݒ�
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeVB);
	//���_�o�b�t�@����
	result = DirectXCommon::GetInstance()->GetDevice()->CreateCommittedResource
		(
			&heapProp, D3D12_HEAP_FLAG_NONE,
			&resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, IID_PPV_ARGS(&vertBuff)
		);
	assert(SUCCEEDED(result));

	//���_�o�b�t�@�]��
	VertexPosNormalUVSkin* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	if(SUCCEEDED(result))
	{
		copy(vertices.begin(), vertices.end(), vertMap);
		vertBuff->Unmap(0, nullptr);
	}
	//���_�o�b�t�@�r���[����
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	vbView.SizeInBytes = sizeVB;
	vbView.StrideInBytes = sizeof(vertices[0]);


	//���_�C���f�b�N�X�S�̂̃T�C�Y
	UINT sizeIB = static_cast<UINT>(sizeof(unsigned short) * indices.size());
	//���\�[�X
	resourceDesc.Width = sizeIB;
	//�C���f�b�N�X�o�b�t�@����
	result = DirectXCommon::GetInstance()->GetDevice()->CreateCommittedResource
		(
			&heapProp, D3D12_HEAP_FLAG_NONE,
			&resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, IID_PPV_ARGS(&indexBuff)
		);
	assert(SUCCEEDED(result));

	//�C���f�b�N�X�o�b�t�@�]��
	unsigned short* indexMap = nullptr;
	result = indexBuff->Map(0, nullptr,(void**)&indexMap);
	if(SUCCEEDED(result))
	{
		copy(indices.begin(),indices.end(),indexMap);
		indexBuff->Unmap(0, nullptr);
	}
	//�C���f�b�N�X�o�b�t�@�r���[(IBV)�̍쐬
	ibView.BufferLocation = indexBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes= sizeIB;

	//�e�N�X�`���f�[�^
	const Image* img = scratchImg.GetImage(0,0,0);
	assert(img);
	//���\�[�X
	CD3DX12_RESOURCE_DESC texResDesc = CD3DX12_RESOURCE_DESC::Tex2D
		(
			metaData.format,
			metaData.width,
			(UINT)metaData.height,
			(UINT)metaData.arraySize,
			(UINT)metaData.mipLevels
		);
	//�q�[�v�v���p�e�B
	CD3DX12_HEAP_PROPERTIES texHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0);
	//�e�N�X�`���o�b�t�@����
	result = DirectXCommon::GetInstance()->GetDevice()->CreateCommittedResource
		(
			&texHeapProp, D3D12_HEAP_FLAG_NONE,
			&texResDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, IID_PPV_ARGS(&texBuff)
		);
	assert(SUCCEEDED(result));

	//�e�N�X�`���o�b�t�@�]��
	result = texBuff->WriteToSubresource
		(
			0, nullptr,img->pixels,
			(UINT)img->rowPitch, (UINT)img->slicePitch
		);
	assert(SUCCEEDED(result));


	//SRV�f�X�N���v�^�q�[�v����
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NumDescriptors = 1;
	result = DirectXCommon::GetInstance()->GetDevice()->CreateDescriptorHeap
		(
			&descHeapDesc, IID_PPV_ARGS(&descHeapsSRV)
		);
	assert(SUCCEEDED(result));

	//�V�F�[�_���\�[�X�r���[
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};	//�ݒ�\����
	D3D12_RESOURCE_DESC resDesc= texBuff->GetDesc();

	srvDesc.Format = resDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension =D3D12_SRV_DIMENSION_TEXTURE2D;	//2D�e�N�X�`��
	srvDesc.Texture2D.MipLevels = 1;

	DirectXCommon::GetInstance()->GetDevice()->CreateShaderResourceView
	(
		texBuff.Get(),	//�r���[�Ɗ֘A�t����o�b�t�@
		&srvDesc,	//�e�N�X�`���ݒ���
		descHeapsSRV->GetCPUDescriptorHandleForHeapStart()	//�q�[�v�̐擪�A�h���X
	);
}

void FbxModelMesh::Draw(ID3D12GraphicsCommandList *commandList, UINT rootParameterIndex)
{
	//���_�o�b�t�@���Z�b�g
	commandList->IASetVertexBuffers(0, 1, &vbView);
	//�C���f�b�N�X�o�b�t�@���Z�b�g
	commandList->IASetIndexBuffer(&ibView);

	//�f�X�N���v�^�q�[�v�̃Z�b�g
	ID3D12DescriptorHeap* ppHeaps[] = {descHeapsSRV.Get()};
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	//�V�F�[�_���\�[�X�r���[���Z�b�g
	commandList->SetGraphicsRootDescriptorTable(rootParameterIndex, descHeapsSRV->GetGPUDescriptorHandleForHeapStart());

	//�`��R�}���h
	commandList->DrawIndexedInstanced((UINT)indices.size(), 1, 0, 0, 0);
}
