#include "Object3D.h"

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

using namespace Microsoft::WRL;
using namespace DirectX;

///<summary>
/// �ÓI�����o�����̎���
///</summary>

ID3D12Device* Object3D::device = nullptr;
Camera* Object3D::camera = nullptr;

ComPtr<ID3D12RootSignature> Object3D::rootsignature;
ComPtr<ID3D12PipelineState> Object3D::pipelinestate;

void Object3D::CreateGraphicsPipeline()
{
	HRESULT result = S_FALSE;

	///���_�V�F�[�_�[file�̓ǂݍ��݂ƃR���p�C��
	ComPtr<ID3DBlob> vsBlob ;			//���_�V�F�[�_�[�I�u�W�F�N�g
	ComPtr<ID3DBlob> psBlob ;			//�s�N�Z���V�F�[�_�[�I�u�W�F�N�g
	ComPtr<ID3DBlob> errorBlob ;		//�G���[�I�u�W�F�N�g

	//���_�V�F�[�_�[�̓ǂݍ��݃R���p�C��
	result = D3DCompileFromFile(
		L"Resources/shaders/FBXVS.hlsl",		//�V�F�[�_�[�t�@�C����
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,	//�C���N���[�h�\�ɂ���
		"main", "vs_5_0",					//�G���g���[�|�C���g���A�V�F�[�_�[���f���w��
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,	//�f�o�b�N�p�ݒ�
		0,
		&vsBlob, &errorBlob);
	//�G���[�Ȃ�
	if(FAILED(result)){
		//errorBlob����G���[���e��string�^�ɃR�s�[
		std::string error;
		error.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
					errorBlob->GetBufferSize(),
					error.begin());
		error += "\n";
		//�G���[���e���o�̓E�B���h�E�ɕ\��
		OutputDebugStringA(error.c_str());
		assert(0);
	}

	//�s�N�Z���V�F�[�_�[�̓ǂݍ��݃R���p�C��
	result = D3DCompileFromFile(
		L"Resources/shaders/FBXPS.hlsl",		//�V�F�[�_�[�t�@�C����
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,	//�C���N���[�h�\�ɂ���
		"main", "ps_5_0",					//�G���g���[�|�C���g���A�V�F�[�_�[���f���w��
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,	//�f�o�b�N�p�ݒ�
		0,
		&psBlob, &errorBlob);
	//�G���[�Ȃ�
	if(FAILED(result)){
		//errorBlob����G���[���e��string�^�ɃR�s�[
		std::string error;
		error.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
					errorBlob->GetBufferSize(),
					error.begin());
		error += "\n";
		//�G���[���e���o�̓E�B���h�E�ɕ\��
		OutputDebugStringA(error.c_str());
		assert(0);
	}


	///���_���C�A�E�g
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
	
		{//xyz���W
			"POSITION",										//�Z�}���e�B�b�N��
			0,												//�����Z�}���e�B�b�N������������Ƃ��Ɏg���C���f�b�N�X
			DXGI_FORMAT_R32G32B32_FLOAT,					//�v�f���ƃr�b�g����\�� (XYZ��3��float�^�Ȃ̂�R32G32B32_FLOAT)
			0,												//���̓X���b�g�C���f�b�N�X
			D3D12_APPEND_ALIGNED_ELEMENT,					//�f�[�^�̃I�t�Z�b�g�l (D3D12_APPEND_ALIGNED_ELEMENT���Ǝ����ݒ�)
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,		//���̓f�[�^��� (�W����D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA)
			0												//��x�ɕ`�悷��C���X�^���X��
		},
		{//�@���x�N�g��
			"NORMAL",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
		{//uv���W
			"TEXCOORD",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
	};


	///<summmary>
	///�O���t�B�b�N�X�p�C�v���C��
	///<summary/>
	
	//�O���t�B�b�N�X�p�C�v���C���ݒ�
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc{};
	//�V�F�[�_�[�ݒ�
	pipelineDesc.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
	pipelineDesc.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());
	
	//�T���v���}�X�N�ݒ�
	pipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;	//�W���ݒ�
	//���X�^���C�U�ݒ� �w�ʃJ�����O	�|���S�����h��Ԃ�	�[�x�N���b�s���O�L��
	pipelineDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//�f�v�X�X�e���V���X�e�[�g�̐ݒ�	(�[�x�e�X�g���s���A�������݋��A�[�x������������΋���)
	pipelineDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	

	//�����_�[�^�[�Q�b�g�̃u�����h�ݒ�
	D3D12_RENDER_TARGET_BLEND_DESC blenddesc{};
	blenddesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;	//RGBA���ׂẴ`�����l����`��
	//���ʐݒ�
	blenddesc.BlendEnable = true;						//�u�����h��L���ɂ���
	blenddesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;		//���Z
	blenddesc.SrcBlendAlpha = D3D12_BLEND_ONE;			//�\�[�X�̒l��100% �g��	(�\�[�X�J���[			 �F ������`�悵�悤�Ƃ��Ă���F)
	blenddesc.DestBlendAlpha = D3D12_BLEND_ZERO;		//�f�X�g�̒l��  0% �g��	(�f�X�e�B�l�[�V�����J���[�F ���ɃL�����o�X�ɕ`����Ă���F)
	//�e��ݒ�
	blenddesc.BlendOp = D3D12_BLEND_OP_ADD;	//�ݒ�
	blenddesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;			//�\�[�X�̒l�� ��% �g��
	blenddesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;	//�f�X�g�̒l�� ��% �g��
	

	//�u�����h�X�e�[�g
	pipelineDesc.BlendState.RenderTarget[0] = blenddesc;
	//�[�x�o�b�t�@�̃t�B�[�}�b�g
	pipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	//���_���C�A�E�g�ݒ�
	pipelineDesc.InputLayout.pInputElementDescs = inputLayout;
	pipelineDesc.InputLayout.NumElements = _countof(inputLayout);
	//�}�`�̌`��ݒ� (�v���~�e�B�u�g�|���W�[)
	pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//���̑��ݒ�
	pipelineDesc.NumRenderTargets = 1;		//�`��Ώۂ͈��
	pipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;	//0~255�w���RGBA
	pipelineDesc.SampleDesc.Count = 1;	//1�s�N�Z���ɂ�1��T���v�����O
	

	//�f�X�N���v�^�����W
	CD3DX12_DESCRIPTOR_RANGE descRangeSRV{};
	descRangeSRV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);	//t0���W�X�^

	//���[�g�V�O�l�`��
	CD3DX12_ROOT_PARAMETER rootparams[2]{};
	//CBV
	rootparams[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
	//SRV
	rootparams[1].InitAsDescriptorTable(1, &descRangeSRV, D3D12_SHADER_VISIBILITY_ALL);

	//�X�^�e�B�b�N�T���v���[
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0);

	//���[�g�V�O�l�`���̐ݒ�
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Init_1_0(_countof(rootparams), rootparams, 1, &samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> rootSigBlob;
	//�o�[�W������������̃V���A���C�Y
	result = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
	//���[�g�V�O�l�`���̐���
	result = device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(rootsignature.ReleaseAndGetAddressOf()));
	if(FAILED(result)) {assert(0);}

	pipelineDesc.pRootSignature = rootsignature.Get();

	//�O���t�B�b�N�X�p�C�v���C���̐���
	result = device->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(pipelinestate.ReleaseAndGetAddressOf()));
	if(FAILED(result)) {assert(0);}
}

void Object3D::Initialize()
{
	HRESULT result;
	//�萔�o�b�t�@�̐���
	result = device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferDataTransform) + 0xff) & ~0xff),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&constBuffTransform)
		);
}

void Object3D::Update()
{
	XMMATRIX matScale, matRot, matTrans;

	//�X�P�[���A��]�A���s�ړ��s��̌v�Z
	matScale = XMMatrixScaling(scale.x, scale.y, scale.z);
	matRot = XMMatrixIdentity();
	matRot *= XMMatrixRotationX(XMConvertToRadians(rotation.x));
	matRot *= XMMatrixRotationY(XMConvertToRadians(rotation.y));
	matRot *= XMMatrixRotationZ(XMConvertToRadians(rotation.z));
	matTrans = XMMatrixTranslation(position.x, position.y, position.z);

	//���[���h�s��̍���
	matWorld = XMMatrixIdentity();	//�ό`�����Z�b�g
	matWorld *= matScale;			//���[���h�s��ɃX�P�[�����O�𔽉f
	matWorld *= matRot;				//���[���h�s��ɉ�]�𔽉f
	matWorld *= matTrans;			//���[���h�s��ɕ��s�ړ��𔽉f

	//�r���[�v���W�F�N�V�����s��
	const XMMATRIX& matViewProjection = camera->GetMatView();
	//���f���̃��b�V���g�����X�t�H�[��
	const XMMATRIX& modelTransform = model->GetModelTransform();
	//�J�������W
	const XMFLOAT3& cameraPos = camera->GetEye();

	HRESULT result;

	//�萔�o�b�t�@�փf�[�^�]��
	ConstBufferDataTransform* constMap = nullptr;
	result = constBuffTransform->Map(0, nullptr, (void**)&constMap);
	if(SUCCEEDED(result))
	{
		constMap->viewproj = matViewProjection;
		constMap->world = modelTransform * matWorld;
		constMap->cameraPos = cameraPos;
		constBuffTransform->Unmap(0, nullptr);
	}
}

void Object3D::Draw(ID3D12GraphicsCommandList* commandList)
{
	if(model == nullptr)
	{
		return ;
	}

	//�p�C�v���C���X�e�[�g�̐ݒ�
	commandList->SetPipelineState(pipelinestate.Get());
	//���[�g�V�O�l�`���̐ݒ�
	commandList->SetGraphicsRootSignature(rootsignature.Get());
	//�v���~�e�B�u�`���ݒ�
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//�萔�o�b�t�@�r�B�[���Z�b�g
	commandList->SetGraphicsRootConstantBufferView(0, constBuffTransform->GetGPUVirtualAddress());

	//���f���`��
	model->Draw(commandList);
}
