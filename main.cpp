#include <Windows.h>

//��{(������)
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

//�A�_�v�^�̗�
#include <vector>
#include <string>

//�}�`�`��
#include <DirectXMath.h>
using namespace DirectX;

//D3D�R���p�C���̃C���N���[�h
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

//�L�[�{�[�h����
#define DIRECTINPUT_VERSION 0x0800	//DirectInput�̃o�[�W�����w��
#include <dinput.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

//DirectXTex����
#include "DirectXTex.h"

//ComPtr�X�}�[�g�|�C���^
#include <wrl.h>
using namespace Microsoft::WRL;


//���_�f�[�^�\����
struct Vertex
{
	XMFLOAT3 pos;	//xyz���W
	XMFLOAT3 normal;//�@���x�N�g��
	XMFLOAT2 uv;	//uv���W
};

//�萔�o�b�t�@�p�f�[�^�\����(�}�e���A��)
struct ConstBufferDataMaterial{
	XMFLOAT4 color;	//�F(RGBA)
};

//�萔�o�b�t�@�p�f�[�^�\����(3D�ϊ��s��
struct ConstBufferDataTransform{
	XMMATRIX mat;	//3D�ϊ��s��
};

//3D�I�u�W�F�N�g�^
struct Object3d
{
	//�萔�o�b�t�@(�s��p)
	ComPtr<ID3D12Resource> constBuffTransform;
	//�萔�o�b�t�@�}�b�v(�s��p)
	ConstBufferDataTransform* constMapTransform;
	//�A�t�B���ϊ�
	XMFLOAT3 scale = {1.0f, 1.0f, 1.0f};
	XMFLOAT3 rotation = {0.0f, 0.0f, 0.0f};
	XMFLOAT3 position = {0.0f, 0.0f, 0.0f};
	//���[���h�ϊ��s��
	XMMATRIX matWorld;
	//�e�I�u�W�F�N�g�ւ̃|�C���^
	Object3d* parent = nullptr;
};



/// <summary>
/// ����
/// </summary>
void InputUpdate(IDirectInputDevice8* devkeyboard, BYTE key[], BYTE oldkey[], int arraysize);
bool Input(const BYTE key[], int KeysName);
bool Output(const BYTE key[], int KeysName);
bool IsInKeyTrigger(const BYTE key[], const BYTE oldkey[], int KeysName);
bool IsOutKeyTrigger(const BYTE key[], const BYTE oldkey[], int KeysName);

//3D�I�u�W�F�N�g������
void InitializeObject3d(Object3d* object, ID3D12Device* device);
void UpdateObject3d(Object3d* object, XMMATRIX& matView, XMMATRIX& matProjection);
void DrawObject3d(Object3d* object, ID3D12GraphicsCommandList* commandList, D3D12_VERTEX_BUFFER_VIEW& vbView, D3D12_INDEX_BUFFER_VIEW& ibView, UINT numIndices);

/// �E�B���h�E�v���V�[�W��
LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	//���b�Z�[�W�ɉ����ăQ�[���ŗL�̏������s��
	switch (msg){
		//�E�B���h�E���j�����ꂽ
	case WM_DESTROY:
		//OS�ɑ΂��āA�A�v���̏I����`����
		PostQuitMessage(0);
		return 0;
	}

	//�W���̃��b�Z�[�W�������s��
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

/// Windows�A�v���ł̃G���g���[�|�C���g(main�֐�)
int WINAPI WinMain(HINSTANCE, HINSTANCE,LPSTR,int)
{
	//�E�B���h�E�T�C�Y
	const int window_width = 1280;	//����
	const int window_height = 720;	//�c��

	//�E�B���h�E�N���X�ݒ�
	WNDCLASSEX w{};
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProc;		//�E�B���h�E�v���V�[�W���ݒ�
	w.lpszClassName = L"DirectXGame";			//�E�B���h�E�N���X��
	w.hInstance = GetModuleHandle(nullptr);		//�E�B���h�E�n���h��
	w.hCursor = LoadCursor(NULL, IDC_ARROW);	//�J�[�\���w��

	//�E�B���h�E�N���X��OS�ɓo�^����
	RegisterClassEx(&w);
	//�E�B���h�E�T�C�Y{ X���W Y���W ���� �c��}
	RECT wrc = {0, 0, window_width, window_height};
	//�����ŃT�C�Y��␳����
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	///�E�B���h�E�I�u�W�F�N�g�̐���
	HWND hwnd = CreateWindow(
		w.lpszClassName,			//�N���X��
		L"DirectXGame",				//�^�C�g���o�[����
		WS_OVERLAPPEDWINDOW,		//�W���I�ȃE�B���h�E�X�^�C��
		CW_USEDEFAULT,				//�\��X���W(OS�ɔC����)
		CW_USEDEFAULT,				//�\��Y���W(OS�ɔC����)
		wrc.right  - wrc.left,		//�E�B���h�E����
		wrc.bottom - wrc.top,		//�E�B���h�E�c��
		nullptr,					//�e�E�B���h�E�n���h��
		nullptr,					//���j���[�n���h��
		w.hInstance,				//�Ăяo���A�v���P�[�V�����n���h��
		nullptr						//�I�v�V����
	);
	//�E�B���h�E��\����Ԃɂ���
	ShowWindow(hwnd, SW_SHOW);

	MSG msg{};	//���b�Z�[�W


	/// <summary>
	/// DirectX12 ���������� ��������
	/// </summary>

	///�f�o�b�N���C���[
#ifdef _DEBUG
	//�f�o�b�N���C���[���I����
	ID3D12Debug* debugController= nullptr;
	if(SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))){
		debugController->EnableDebugLayer();
	}
#endif // _DEBUG


	/// <summary>
	/// �������ϐ�
	/// </summary>
	HRESULT result;
	ComPtr<ID3D12Device> device = nullptr;
	ComPtr<IDXGIFactory7> dxgiFactory= nullptr;
	ComPtr<IDXGISwapChain4> swapChain = nullptr;
	ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	ComPtr<ID3D12DescriptorHeap> rtvHeap = nullptr;

	///�A�_�v�^��
	//DXGI�t�@�N�g���[
	result = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	assert(SUCCEEDED(result));

	//�A�_�v�^�[�̗񋓗p
	std::vector<ComPtr<IDXGIAdapter4>> adapters;
	//�����ɓ���̖��O�����A�_�v�^�[�I�u�W�F�N�g������
	ComPtr<IDXGIAdapter4> tmpAdapter = nullptr;

	//�p�t�H�[�}���X���������̂��珇�ɁA���ׂẴA�_�v�^��񋓂���
	for(UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&tmpAdapter)) != DXGI_ERROR_NOT_FOUND; i++)
	{
		//���I�z��ɒǉ�����
		adapters.push_back(tmpAdapter);
	}

	///�A�_�v�^�̑I��
	//�œ|�ȃA�_�v�^��I�ʂ���
	for(size_t i = 0; i < adapters.size(); i++)
	{
		DXGI_ADAPTER_DESC3 adapterDesc;
		//�A�_�v�^�[�̏����擾����
		adapters[i]->GetDesc3(&adapterDesc);

		//�\�t�g�E�F�A�f�o�C�X�����
		if(!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE))
		{
			//�f�o�C�X���̗p���ă��[�v�𔲂���
			tmpAdapter = adapters[i].Get();
			break;
		}
	}

	///�f�o�C�X�̐���(1�Q�[���Ɉ��)
	//�Ή����x���̔z��
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
		//�̗p�����A�_�v�^�[�Ńf�o�C�X�𐶐�
		result = D3D12CreateDevice(tmpAdapter.Get(), levels[i], IID_PPV_ARGS(&device));
		assert(SUCCEEDED(result));
		if(result == S_OK)
		{
			//�f�o�C�X�𐶐��o�������_�Ń��[�v�𔲂���
			featureLevel = levels[i];
			break;
		}
	}

	///�R�}���h���X�g(GPU�ɁA�܂Ƃ߂Ė��߂𑗂邽�߂̃R�}���h)
	//�R�}���h�A���P�[�^�𐶐�
	result = device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&commandAllocator)
	);
	assert(SUCCEEDED(result));

	//�R�}���h���X�g�𐶐�
	result = device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		commandAllocator.Get(),
		nullptr,
		IID_PPV_ARGS(&commandList)
	);
	assert(SUCCEEDED(result));


	///�R�}���h�L���[(�R�}���h���X�g��GPU�ɏ��ʎ��s�����Ă����d�g��)
	//�R�}���h�L���[�̐ݒ�
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	//�R�}���h�L���[�𐶐�
	result = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	assert(SUCCEEDED(result));


	///�X���b�v�`�F�[��(�t�����g�o�b�t�@�A�o�b�N�o�b�t�@�����ւ��ăp���p����������)
	//�ݒ�
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = window_width;
	swapChainDesc.Height = window_height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;				//�F��񏑎�
	swapChainDesc.SampleDesc.Count = 1;								//�}���`�T���v�����Ȃ�
	swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;				//�o�b�N�o�b�t�@�p
	swapChainDesc.BufferCount = 2;									//�o�b�t�@�����ɐݒ�
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;		//�t���b�v��͔j��
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	//����
	//IDXGISwapChain1��ComPtr�p��
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
	//��������IDXGISwapChain1�̃I�u�W�F�N�g��IDXGISwapChain4�ɕϊ�����
	swapchain1.As(&swapChain);

	///�����_�[�^�[�Q�b�g�r���[(�o�b�N�o�b�t�@��`��L�����o�X�Ƃ��Ĉ����I�u�W�F�N�g)
	//�f�X�N���v�^�q�[�v����(�����_�[�^�[�Q�b�g�r���[�̓f�X�N���v�^�q�[�v�ɐ�������̂ŏ���)
	//�ݒ�
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;		//�����_�[�^�[�Q�b�g�r���[
	rtvHeapDesc.NumDescriptors = swapChainDesc.BufferCount;	//�\���̓��(�_�u���o�b�t�@�����O)
	//����
	device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));

	//�o�b�N�o�b�t�@(�X���b�v�`�F�[�����Ő������ꂽ�o�b�N�o�b�t�@�̃A�h���X���e�p)
	std::vector<ComPtr<ID3D12Resource>> backBuffers;
	backBuffers.resize(swapChainDesc.BufferCount);

	//�����_�[�^�[�Q�b�g�r���[(RTV)����
	//�X���b�v�`�F�[���̂��ׂẴo�b�t�@�ɂ��ď�������
	for(size_t i = 0; i < backBuffers.size(); i++)
	{
		//�X���b�v�`�F�[������o�b�t�@���擾
		swapChain->GetBuffer((UINT)i, IID_PPV_ARGS(&backBuffers[i]));
		//�f�X�N���v�^�q�[�v�̃n���h�����擾
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
		//�\�����ŃA�h���X�������
		rtvHandle.ptr += i * device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);
		//�����_�[�^�[�Q�b�g�r���[�̐ݒ�
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		//�V�F�[�_�[�̌v�Z���ʂ�SRGB�ɕϊ����ď�������
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		//�����_�[�^�[�Q�b�g�r���[�̐���
		device->CreateRenderTargetView(backBuffers[i].Get(), &rtvDesc, rtvHandle);
	}

	///�[�x�o�b�t�@�̃��\�[�X(�e�N�X�`���̈��)
	//�ݒ�
	D3D12_RESOURCE_DESC depthResourceDesc{};
	depthResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResourceDesc.Width = window_width;		//�����_�[�^�[�Q�b�g�ɍ��킹��
	depthResourceDesc.Height = window_height;	//�����_�[�^�[�Q�b�g�ɍ��킹��
	depthResourceDesc.DepthOrArraySize = 1;
	depthResourceDesc.Format = DXGI_FORMAT_D32_FLOAT;	//�[�x�l�t�H�[�}�b�g
	depthResourceDesc.SampleDesc.Count = 1;
	depthResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;	//�f�v�X�X�e���V��
	//�[�x�l�p�q�[�v�v���p�e�B
	D3D12_HEAP_PROPERTIES depthHeapProp{};
	depthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	//�[�x�l�̃N���A�ݒ�
	D3D12_CLEAR_VALUE idepthClearValue{};
	idepthClearValue.DepthStencil.Depth = 1.0f;		//�[�x�l1.0f(�ő�l)�ŃN���A
	idepthClearValue.Format = DXGI_FORMAT_D32_FLOAT;//�[�x�l�t�H�[�}�b�g
	//���\�[�X����
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
	//�[�x�r���[�p�f�X�N���v�^�q�[�v�쐬(DSV)
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
	dsvHeapDesc.NumDescriptors = 1;			//�[�x�r���[�͈��
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;	//�f�v�X�X�e���V���r���[
	ComPtr<ID3D12DescriptorHeap> dsvHeap = nullptr;
	result = device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap));
	assert(SUCCEEDED(result));
	//�[�x�r���[�쐬
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	device->CreateDepthStencilView(
		depthBuff.Get(),
		&dsvDesc,
		dsvHeap->GetCPUDescriptorHandleForHeapStart()
	);


	///�t�F���X(CPU��GPU�œ������Ƃ邽�߂̎d�g��)
	//����
	ComPtr<ID3D12Fence> fence;
	UINT64 fenceVal = 0;
	result = device->CreateFence(fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(result));

	///DirectInPut
	//������ (�����͕��@�ǉ��ł����̃I�u�W�F�N�g�͈�̂�)
	IDirectInput8* directInput = nullptr;
	result = DirectInput8Create(
		w.hInstance, 
		DIRECTINPUT_VERSION, 
		IID_IDirectInput8,
		(void**)&directInput, 
		nullptr
	);
	assert(SUCCEEDED(result));

	//�L�[�{�[�h�f�o�C�X�̐��� (GUID_Joystick (�W���C�X�e�b�N)�A GUID_SysMouse (�}�E�X))
	IDirectInputDevice8* keyboard = nullptr;
	result = directInput->CreateDevice(
		GUID_SysKeyboard,
		&keyboard,
		NULL
	);
	assert(SUCCEEDED(result));

	//���̓f�[�^�`���̃Z�b�g (���̓f�o�C�X�̎�ނɂ���āA���炩���߉���ނ��p�ӂ���)
	result = keyboard->SetDataFormat(&c_dfDIKeyboard);	//�W���`��
	assert(SUCCEEDED(result));

	//�r���I���䃌�x���̃Z�b�g
	//DISCL_FOREGROUND		��ʂ���O�ɂ���ꍇ�̂ݓ��͂��󂯕t����
	//DISCL_NONEXCLUSIVE	�f�o�C�X�����̃A�v�������Ő�L���Ȃ�
	//DISCL_NOWINKEY		Windows�L�[�𖳌��ɂ���
	result = keyboard->SetCooperativeLevel(
		hwnd,
		DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY
	);
	assert(SUCCEEDED(result));




	/// <summary>
	/// DirectX12 ���������� �����܂�
	/// </summary>
	

	
	/// <summary>
	/// DirectX12 �`�揉�������� ��������
	/// </summary>
	 
	///���_�f�[�^
	Vertex vertices[] = 
	{
		//�O
		{{-5.0f, -5.0f, -5.0f}, {}, {0.0f, 1.0f}},
		{{-5.0f, +5.0f, -5.0f}, {}, {0.0f, 0.0f}},
		{{+5.0f, -5.0f, -5.0f}, {}, {1.0f, 1.0f}},
		{{+5.0f, +5.0f, -5.0f}, {}, {1.0f, 0.0f}},
		//��
		{{-5.0f, -5.0f, +5.0f}, {}, {0.0f, 1.0f}},
		{{-5.0f, +5.0f, +5.0f}, {}, {0.0f, 0.0f}},
		{{+5.0f, -5.0f, +5.0f}, {}, {1.0f, 1.0f}},
		{{+5.0f, +5.0f, +5.0f}, {}, {1.0f, 0.0f}},
		//��
		{{-5.0f, -5.0f, -5.0f}, {}, {0.0f, 1.0f}},
		{{-5.0f, -5.0f, +5.0f}, {}, {0.0f, 0.0f}},
		{{-5.0f, +5.0f, -5.0f}, {}, {1.0f, 1.0f}},
		{{-5.0f, +5.0f, +5.0f}, {}, {1.0f, 0.0f}},
		//�E
		{{+5.0f, -5.0f, -5.0f}, {}, {0.0f, 1.0f}},
		{{+5.0f, -5.0f, +5.0f}, {}, {0.0f, 0.0f}},
		{{+5.0f, +5.0f, -5.0f}, {}, {1.0f, 1.0f}},
		{{+5.0f, +5.0f, +5.0f}, {}, {1.0f, 0.0f}},
		//��
		{{-5.0f, -5.0f, -5.0f}, {}, {0.0f, 1.0f}},
		{{+5.0f, -5.0f, -5.0f}, {}, {0.0f, 0.0f}},
		{{-5.0f, -5.0f, +5.0f}, {}, {1.0f, 1.0f}},
		{{+5.0f, -5.0f, +5.0f}, {}, {1.0f, 0.0f}},
		//��
		{{-5.0f, +5.0f, -5.0f}, {}, {0.0f, 1.0f}},
		{{+5.0f, +5.0f, -5.0f}, {}, {0.0f, 0.0f}},
		{{-5.0f, +5.0f, +5.0f}, {}, {1.0f, 1.0f}},
		{{+5.0f, +5.0f, +5.0f}, {}, {1.0f, 0.0f}},
	};

	///�C���f�b�N�X�f�[�^
	uint16_t indices[] = 
	{
		//�O
		0, 1, 2,
		2, 1, 3,
		//��
		5, 4, 6,
		5, 6, 7,
		//��
		8, 9, 10,
		10, 9, 11,
		//�E
		13, 12, 14,
		13, 14, 15,
		//��
		16, 17, 18,
		18, 17, 19,
		//��
		21, 20, 22,
		21, 22, 23,
	};

	///�@���v�Z
	for(int i = 0; i < _countof(indices)/3; i++)
	{//�O�p�`����ƂɌv�Z���Ă���
		//�O�p�`�ɃC���f�b�N�X�����o���āA�ꎞ�I�ȕϐ�������
		uint16_t index0 = indices[i*3+0];
		uint16_t index1 = indices[i*3+1];
		uint16_t index2 = indices[i*3+2];
		//�O�p�`���\�����钸�_���W���x�N�g���ɑ��
		XMVECTOR p0 = XMLoadFloat3(&vertices[index0].pos);
		XMVECTOR p1 = XMLoadFloat3(&vertices[index1].pos);
		XMVECTOR p2 = XMLoadFloat3(&vertices[index2].pos);
		//p0->p1�x�N�g���Ap0->p2�x�N�g���̌v�Z	(�x�N�g�����Z)
		XMVECTOR v1 = XMVectorSubtract(p1,p0);
		XMVECTOR v2 = XMVectorSubtract(p2,p0);
		//�O�ς͗������琂���ȃx�N�g��
		XMVECTOR normal = XMVector3Cross(v1,v2);
		//���K��(������1�ɂ���)
		normal = XMVector3Normalize(normal);
		//���߂��@���𒸓_�f�[�^�ɑ��
		XMStoreFloat3(&vertices[index0].normal,normal);
		XMStoreFloat3(&vertices[index1].normal,normal);
		XMStoreFloat3(&vertices[index2].normal,normal);
	}


	//���_�f�[�^�S�̂̃T�C�Y = ���_�f�[�^����̃T�C�Y * ���_�f�[�^�̗v�f��
	UINT sizeVB = static_cast<UINT>(sizeof(vertices[0]) * _countof(vertices));

	///���_�o�b�t�@�̊m��
	//�q�[�v�ݒ�
	D3D12_HEAP_PROPERTIES heapProp{};			//�q�[�v�ݒ�
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;		//GPU�ւ̓]��
	//���\�[�X�ݒ�
	D3D12_RESOURCE_DESC resDesc{};				//���\�[�X�ݒ�
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = sizeVB;			//���_�f�[�^�S�̂̃T�C�Y
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	//����
	ComPtr<ID3D12Resource> vertBuff;
	result = device->CreateCommittedResource(
		&heapProp,				//�q�[�v�ݒ�
		D3D12_HEAP_FLAG_NONE,
		&resDesc,				//���\�[�X�ݒ�
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff)
	);
	assert(SUCCEEDED(result));


	///���_�o�b�t�@�ւ̃f�[�^�]��
	//GPU��̃o�b�t�@�ɑΉ��������z������(���C����������)���擾
	Vertex* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	assert(SUCCEEDED(result));
	//�S���_�ɑ΂���
	for(int i = 0; i < _countof(vertices); i++)
	{
		vertMap[i] = vertices[i];	//���W�R�s�[
	}
	//�q�������
	vertBuff->Unmap(0, nullptr);


	///���_�o�b�t�@�r���[�̍쐬(GPU�ɒ��_�o�b�t�@��������I�u�W�F�N�g)
	//�쐬
	D3D12_VERTEX_BUFFER_VIEW vbView{};
	//GPU���z�A�h���X
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	//���_�o�b�t�@�̃T�C�Y
	vbView.SizeInBytes = sizeVB;
	//���_����̃f�[�^�T�C�Y
	vbView.StrideInBytes = sizeof(vertices[0]);


	///���_�C���f�b�N�X
	//�C���f�b�N�X�f�[�^�S�̂̃T�C�Y
	UINT sizeIB = static_cast<UINT>(sizeof(uint16_t) * _countof(indices));

	///�C���f�b�N�X�o�b�t�@�̐���
	//���\�[�X�ݒ�
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = sizeIB;	//�C���f�b�N�X��񂪓��镪�̃T�C�Y
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//����
	ComPtr<ID3D12Resource> indexBuff;
	result = device->CreateCommittedResource(
		&heapProp,				//�q�[�v�ݒ�
		D3D12_HEAP_FLAG_NONE,
		&resDesc,				//���\�[�X�ݒ�
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBuff)
	);
	assert(SUCCEEDED(result));

	///�C���f�b�N�X�o�b�t�@�ւ̃f�[�^�]��
	//�}�b�s���O
	uint16_t* indexMap = nullptr;
	result = indexBuff->Map(0, nullptr, (void**)&indexMap);
	assert(SUCCEEDED(result));
	//�S�C���f�b�N�X�ɑ΂���
	for(int i = 0; i < _countof(indices); i++)
	{
		indexMap[i] = indices[i];
	}
	//�}�b�s���O����
	indexBuff->Unmap(0, nullptr);


	///�C���f�b�N�X�o�b�t�@�r���[�̍쐬(GPU�ɃC���f�b�N�X�o�b�t�@��������I�u�W�F�N�g)
	D3D12_INDEX_BUFFER_VIEW ibView{};
	ibView.BufferLocation = indexBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeIB;



	///���_�V�F�[�_�[file�̓ǂݍ��݂ƃR���p�C��
	ComPtr<ID3DBlob> vsBlob ;			//���_�V�F�[�_�[�I�u�W�F�N�g
	ComPtr<ID3DBlob> psBlob ;			//�s�N�Z���V�F�[�_�[�I�u�W�F�N�g
	ComPtr<ID3DBlob> errorBlob ;		//�G���[�I�u�W�F�N�g

	//���_�V�F�[�_�[�̓ǂݍ��݃R���p�C��
	result = D3DCompileFromFile(
		L"BasicVS.hlsl",		//�V�F�[�_�[�t�@�C����
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
		L"BasicPS.hlsl",		//�V�F�[�_�[�t�@�C����
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



	///�萔�o�b�t�@ Color
	//GPU���\�[�X�|�C���^
	ComPtr<ID3D12Resource> constBufferMaterial ;
	//�}�b�s���O�p�|�C���^
	ConstBufferDataMaterial* constMapMaterial = nullptr;
	{
		//�����p�̐ݒ�
		//�q�[�v�ݒ�
		D3D12_HEAP_PROPERTIES cbHeapProp{};
		cbHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;	//GPU�ւ̓]���p
		//���\�[�X�ݒ�
		D3D12_RESOURCE_DESC cbResourceDesc{};
		cbResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		cbResourceDesc.Width = (sizeof(ConstBufferDataMaterial) + 0xff) & ~0xff;	//256�o�C�g�A���C�����g
		cbResourceDesc.Height = 1;
		cbResourceDesc.DepthOrArraySize = 1;
		cbResourceDesc.MipLevels = 1;
		cbResourceDesc.SampleDesc.Count = 1;
		cbResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		//����
		result = device->CreateCommittedResource(
			&cbHeapProp,			//�q�[�v�ݒ�
			D3D12_HEAP_FLAG_NONE,
			&cbResourceDesc,		//���\�[�X�ݒ�
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&constBufferMaterial)
		);
		assert(SUCCEEDED(result));

		///�萔�o�b�t�@�̃}�b�s���O(GPU��VRAM���ACPU�̃��C���������ɘA��)
		result = constBufferMaterial->Map(0, nullptr, (void**)&constMapMaterial);
		assert(SUCCEEDED(result));
	
		///�萔�o�b�t�@�ւ̃f�[�^�]��
		//�l���������ނƎ����I�ɓ]�������
		XMFLOAT4 color = {1.0f, 1.0f, 1.0f, 1.0f};
		constMapMaterial->color = color;	//RGBA�Ŕ������̐�

		//Unmap����ƘA������������� (�萔�o�b�t�@�͌p���I�ɒl������������p�r�������̂ŁA���̂܂܂ɂ��Ă悢)
		//constBufferMaterial->Unmap(0, nullptr);
	}


	//�V�F�[�_���\�[�X�r���[�̃f�X�N���v�^�q�[�v
	ComPtr<ID3D12DescriptorHeap> srvHeap = nullptr;
	
		///�摜�t�@�C���̗p��
		TexMetadata metadata{};
		TexMetadata metadata2{};
		ScratchImage scratchImg{};
		ScratchImage scratchImg2{};
		//WIC�e�N�X�`���f�[�^�̃��[�h
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

		//�~�b�v�}�b�v�̐���
		ScratchImage mipChain{};
		//����
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
		//����
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

		//�t�H�[�}�b�g������������
		//�ǂݍ��񂾃f�B�t���[�Y�e�N�X�`����SRGB�Ƃ��Ĉ���
		metadata.format = MakeSRGB(metadata.format);
		metadata2.format = MakeSRGB(metadata2.format);


		///�e�N�X�`���o�b�t�@�ݒ�
		//�q�[�v�ݒ�
		D3D12_HEAP_PROPERTIES textureHandleProp{};
		textureHandleProp.Type = D3D12_HEAP_TYPE_CUSTOM;
		textureHandleProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
		textureHandleProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
		//���\�[�X�ݒ�
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

		//�e�N�X�`���o�b�t�@�̐���
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


		//�e�N�X�`���o�b�t�@�ւ̃f�[�^�]��
		//�S�~�b�v�}�b�v�ɂ���
		for(size_t i = 0; i < metadata.mipLevels; i++)
		{
			//�~�b�v�}�b�v���x�����w�肵�ăC���[�W���擾
			const Image* img = scratchImg.GetImage(i, 0, 0);
			//�e�N�X�`���o�b�t�@�Ƀf�[�^�]��
			result = texBuff->WriteToSubresource(
				(UINT)i,				
				nullptr,				//�S�̈�փR�s�[
				img->pixels,			//���f�[�^�A�h���X
				(UINT)img->rowPitch,	//�ꃉ�C���T�C�Y
				(UINT)img->slicePitch	//�ꖇ�T�C�Y
			);
			assert(SUCCEEDED(result));
		}
		for(size_t i = 0; i < metadata2.mipLevels; i++)
		{
			//�~�b�v�}�b�v���x�����w�肵�ăC���[�W���擾
			const Image* img = scratchImg2.GetImage(i, 0, 0);
			//�e�N�X�`���o�b�t�@�Ƀf�[�^�]��
			result = texBuff2->WriteToSubresource(
				(UINT)i,				
				nullptr,				//�S�̈�փR�s�[
				img->pixels,			//���f�[�^�A�h���X
				(UINT)img->rowPitch,	//�ꃉ�C���T�C�Y
				(UINT)img->slicePitch	//�ꖇ�T�C�Y
			);
			assert(SUCCEEDED(result));
		}



		///�f�X�N���v�^�q�[�v����
		//SRV�̍ő��
		const size_t kMaxSRVCount = 2056;
		//�f�X�N���v�^�q�[�v�̐ݒ�
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;	//�V�F�[�_�[���猩����悤��
		srvHeapDesc.NumDescriptors = kMaxSRVCount;
		//�ݒ�����Ƃ�SRV�p�f�X�N���v�^�q�[�v�𐶐�
		result = device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvHeap));
		assert(SUCCEEDED(result));

		///�f�X�N���v�^�n���h��
		//SRV�q�[�v�̐擪�n���h�����擾
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = srvHeap->GetCPUDescriptorHandleForHeapStart();


		///�V�F�[�_���\�[�X�r���[�̍쐬
		//�ݒ�
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};	//�ݒ�\����
		srvDesc.Format = textureResourceDesc.Format;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;	//2D�e�N�X�`��
		srvDesc.Texture2D.MipLevels = textureResourceDesc.MipLevels;

		//�n���h���̎w���ʒu�ɃV�F�[�_�[���\�[�X�r���[�̍쐬
		device->CreateShaderResourceView(texBuff.Get(), &srvDesc, srvHandle);

		//��n���h����i�߂�
		UINT incrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		srvHandle.ptr += incrementSize;

		///�V�F�[�_���\�[�X�r���[�̍쐬
		//�ݒ�
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};	//�ݒ�\����
		srvDesc2.Format = textureResourceDesc2.Format;
		srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;	//2D�e�N�X�`��
		srvDesc2.Texture2D.MipLevels = textureResourceDesc2.MipLevels;

		//�n���h���̎w���ʒu�ɃV�F�[�_�[���\�[�X�r���[�̍쐬
		device->CreateShaderResourceView(texBuff2.Get(), &srvDesc2, srvHandle);
	


	//�萔�o�b�t�@ Mat	
	//�������e
	XMMATRIX matProjection;
	//�r���[�ϊ��s��
	XMMATRIX matView;
	XMFLOAT3 eye = {0.0f, 0.0f, -100.0f};	//���_���W
	XMFLOAT3 target= {0, 0, 0};//�����_���W
	XMFLOAT3 up = {0, 1, 0};	//������x�N�g��

	//3D�I�u�W�F�N�g��
	const size_t kObjectCount = 50;
	//3d�I�u�W�F�N�g�̔z��
	Object3d object3ds[kObjectCount];
	
	{
		//�ݒ�
		//�q�[�v
		D3D12_HEAP_PROPERTIES cbHeapProp{};
		cbHeapProp.Type= D3D12_HEAP_TYPE_UPLOAD;	//GPU�]���p
		//���\�[�X
		D3D12_RESOURCE_DESC cbResourceDesc{};
		cbResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		cbResourceDesc.Width = (sizeof(ConstBufferDataTransform)+0xff) & ~0xff;	//256�o�C�g�A���C�����g
		cbResourceDesc.Height = 1;
		cbResourceDesc.DepthOrArraySize = 1;
		cbResourceDesc.MipLevels = 1;
		cbResourceDesc.SampleDesc.Count = 1;
		cbResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		
		//�z����̑S�I�u�W�F�N�g�ɑ΂���
		for(size_t i = 0; i < _countof(object3ds); i++)
		{
			//������
			InitializeObject3d(&object3ds[i], device.Get());

			//�e�q�\����
			//�擪�ȊO
			if(i > 0)
			{
				//��O�̃I�u�W�F�N�g��e�Ƃ���
				//object3ds[i].parent = &object3ds[i - 1];

				//Scale
				object3ds[i].scale = {0.9f, 0.9f, 0.9f};
				//rotation
				object3ds[i].rotation = {0.0f, 0.0f, XMConvertToRadians(30.0f)};
				//position
				object3ds[i].position = {0.0f, 0.0f, 8.0f};
			}
		}

	

		//�������e
		matProjection = XMMatrixPerspectiveFovLH(
			XMConvertToRadians(45.0f),	//�㉺��p45��
			(float)1280 / 720,			//aspect��(��ʉ���/��ʏc��)
			0.1f, 1000.0f				//�O�[�A���[
		);

		//�r���[�ϊ��s��
		matView = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));	
	}



	///���[�g�p�����[�^
	//�f�X�N���v�^�����W�̐ݒ�
	D3D12_DESCRIPTOR_RANGE descriptorRange{};
	descriptorRange.NumDescriptors = 1;			//��x�̕`��Ɏg���e�N�X�`�����ꖇ�Ȃ̂�1
	descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange.BaseShaderRegister = 0;		//�e�N�X�`�����W�X�^0��
	descriptorRange.OffsetInDescriptorsFromTableStart =D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	//�ݒ�
	D3D12_ROOT_PARAMETER rootParam[3] = {};
	//�萔�o�b�t�@ 0��
	rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;	//�萔�o�b�t�@�r���[
	rootParam[0].Descriptor.ShaderRegister = 0;					//�萔�o�b�t�@�ԍ�0
	rootParam[0].Descriptor.RegisterSpace = 0;						//�f�t�H���g�l
	rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;	//���ׂẴV�F�[�_�[���猩����
	//�e�N�X�`�����W�X�^ 0��
	rootParam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;	//���
	rootParam[1].DescriptorTable.pDescriptorRanges = &descriptorRange;			//�f�X�N���v�^�����W0
	rootParam[1].DescriptorTable.NumDescriptorRanges = 1;					//�f�X�N���v�^�����W��
	rootParam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;			//�S�ẴV�F�[�_�[���猩����
	//�萔�o�b�t�@ 1��
	rootParam[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;	//�萔�o�b�t�@�r���[
	rootParam[2].Descriptor.ShaderRegister = 1;					//�萔�o�b�t�@�ԍ�1
	rootParam[2].Descriptor.RegisterSpace = 0;					//�f�t�H���g�l
	rootParam[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;	//�S�ẴV�F�[�_���猩����


	///<summmary>
	///�O���t�B�b�N�X�p�C�v���C��
	///<summary/>
	
	//�O���t�B�b�N�X�p�C�v���C���ݒ�
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc{};
	//�V�F�[�_�[�ݒ�
	pipelineDesc.VS.pShaderBytecode = vsBlob->GetBufferPointer();
	pipelineDesc.VS.BytecodeLength  = vsBlob->GetBufferSize();
	pipelineDesc.PS.pShaderBytecode = psBlob->GetBufferPointer();
	pipelineDesc.PS.BytecodeLength  = psBlob->GetBufferSize();
	//�T���v���}�X�N�ݒ�
	pipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;	//�W���ݒ�
	//���X�^���C�U�ݒ�
	pipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;	//�w�ʃJ�����O
	pipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;	//�|���S�����h��Ԃ���
	pipelineDesc.RasterizerState.DepthClipEnable = true;			//�[�x�N���b�s���O��L����
	//�u�����h�X�e�[�g
	//�����_�[�^�[�Q�b�g�̃u�����h�ݒ�
	D3D12_RENDER_TARGET_BLEND_DESC& blenddesc = pipelineDesc.BlendState.RenderTarget[0];
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
	//���_���C�A�E�g�ݒ�
	pipelineDesc.InputLayout.pInputElementDescs = inputLayout;
	pipelineDesc.InputLayout.NumElements = _countof(inputLayout);
	//�}�`�̌`��ݒ� (�v���~�e�B�u�g�|���W�[)
	pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//���̑��ݒ�
	pipelineDesc.NumRenderTargets = 1;		//�`��Ώۂ͈��
	pipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;	//0~255�w���RGBA
	pipelineDesc.SampleDesc.Count = 1;	//1�s�N�Z���ɂ�1��T���v�����O
	//�f�v�X�X�e���V���X�e�[�g�̐ݒ�
	pipelineDesc.DepthStencilState.DepthEnable= true;		//�[�x�e�X�g���s��
	pipelineDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;	//�������݋���
	pipelineDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;	//��������΍��i
	pipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;	//�[�x�l�t�H�[�}�b�g


	///�e�N�X�`���T���v���[
	//�ݒ�
	D3D12_STATIC_SAMPLER_DESC samplerDesc{};
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;		//���J��Ԃ�(�^�C�����O)
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;		//�c�J��Ԃ�(�^�C�����O)
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;		//���s�J��Ԃ�(�^�C�����O)
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;	//�{�[�_�[�̎��͍�
	samplerDesc.Filter= D3D12_FILTER_MIN_MAG_MIP_LINEAR;		//�S�ĂŃ��j�A���
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;			//�~�b�v�}�b�v�ő�l
	samplerDesc.MinLOD = 0.0f;						//�~�b�v�}�b�v�ŏ��l
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	//�s�N�Z���V�F�[�_����̂ݎg�p�\



	//���[�g�V�O�l�`�� (�e�N�X�`���A�_���o�b�t�@�ȂǃV�F�[�_�[�ɓn�����\�[�X�����܂Ƃ߂��I�u�W�F�N�g)
	//���[�g�V�O�l�`���̐���
	ComPtr<ID3D12RootSignature> rootSignature;
	//�ݒ�
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.pParameters = rootParam;	//���[�g�p�����[�^�̐擪�A�h���X
	rootSignatureDesc.NumParameters = _countof(rootParam);		//���[�g�p�����[�^��
	rootSignatureDesc.pStaticSamplers = &samplerDesc;
	rootSignatureDesc.NumStaticSamplers= 1;
	//�V���A���C�Y
	ComPtr<ID3DBlob> rootSigBlob;
	result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
	assert(SUCCEEDED(result));
	result = device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(result));
	//�p�C�v���C���Ƀ��[�g�V�O�l�`�����Z�b�g
	pipelineDesc.pRootSignature = rootSignature.Get();

	//�p�C�v���C���X�e�[�g (�O���t�B�b�N�X�p�C�v���C���̐ݒ���܂Ƃ߂��̂��p�C�v���C���X�e�[�g�I�u�W�F�N�g(PSO))
	//�p�C�v���C���X�e�[�g�̐���
	ComPtr<ID3D12PipelineState> pipelineState ;
	result = device->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&pipelineState));
	assert(SUCCEEDED(result));



	/// <summary>
	/// DirectX12 �`�揉�������� �����܂�
	/// </summary>
	 

	float angle = 0.0f;//�J�����̉�]�p
	bool IsTexture = false;

	//�S�L�[�̓��͏�Ԃ��擾����
	const int KeyNum = 256;
	BYTE key[KeyNum] = {};
	BYTE oldkeys[KeyNum] = {};

	/// <summary>
	/// �Q�[�����[�v
	/// </summary>
	while(true)
	{
		//���b�Z�[�W������?
		if(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);	//�L�[���̓��b�Z�[�W�̏���
			DispatchMessage(&msg);	//�v���V�[�W���Ƀ��b�Z�[�W�𑗂�
		}

		//�~�{�^���ŏI�����b�Z�[�W��������Q�[�����[�v�𔲂���
		if(msg.message == WM_QUIT)
		{
			break;
		}


		/// <summary>
		/// DirectX12 ���t���[������ ��������
		/// </summary>

		///�L�[�{�[�h���̎擾�J�n
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

			//angle���W�A������y���܂��ɉ�]�A���a��-100
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

		//�X�V����
		for(size_t i = 0; i < _countof(object3ds); i++)
		{
			UpdateObject3d(&object3ds[i], matView, matProjection);
		}



		///���\�[�X�o���A01
		//�o�b�N�o�b�t�@�̔ԍ����擾(�_�u���o�b�t�@�Ȃ̂� 0 or 1)
		UINT bbIndex = swapChain->GetCurrentBackBufferIndex();
		
		//1. ���\�[�X�o���A�ŏ������݉\�ɕύX
		D3D12_RESOURCE_BARRIER barrierDesc{};
		barrierDesc.Transition.pResource = backBuffers[bbIndex].Get();					//�o�b�N�o�b�t�@���w��
		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;			//�\����Ԃ���
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;	//�`���Ԃ�
		commandList->ResourceBarrier(1, &barrierDesc);


		///�`���w��R�}���h
		//2. �`���̕ύX
		//�����_�[�^�[�Q�b�g�r���[�̃n���h���擾
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
		rtvHandle.ptr += bbIndex * device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);
		//�[�x�X�e���V���r���[�p�f�X�N���v�^�q�[�v�̃n���h���擾
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();
		commandList->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);


		///��ʃN���A�R�}���h
		//3. ��ʃN���A
		float clearColor[] = {0.1f, 0.25f, 0.5f, 0.0f};
		//�F�N���A
		commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		//�[�x�N���A
		commandList->ClearDepthStencilView(dsvHandle,D3D12_CLEAR_FLAG_DEPTH,1.0f, 0, 0, nullptr);


		///�`��R�}���h
		//4. �`��R�}���h ��������

		///<summary>
		///	�O���t�B�b�N�X�R�}���h
		///<summary/>
		
		///�r���[�|�[�g
		//�ݒ�R�}���h
		D3D12_VIEWPORT viewport{};
		viewport.Width = window_width;
		viewport.Height = window_height;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		//�r���[�|�[�g�ݒ�R�}���h���R�}���h���X�g�ɐς�
		commandList->RSSetViewports(1, &viewport);
		 
		///�V�U�[��`
		//�ݒ�
		D3D12_RECT scissorRect{};
		scissorRect.left = 0;
		scissorRect.right = scissorRect.left + window_width;
		scissorRect.top = 0;
		scissorRect.bottom = scissorRect.top + window_height;
		//�V�U�[��`�ݒ�R�}���h���A�R�}���h���X�g�ɐς�
		commandList->RSSetScissorRects(1, &scissorRect);

		///�p�C�v���C���X�e�[�g�ƃ��[�g�V�O�l�`���̐ݒ�R�}���h
		commandList->SetPipelineState(pipelineState.Get());
		commandList->SetGraphicsRootSignature(rootSignature.Get());

		///�v���~�e�B�u�`��
		//�v���~�e�B�u�`��̐ݒ�R�}���h
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		///���_�o�b�t�@�r���[
		//���_�o�b�t�@�r���[�̐ݒ�R�}���h
		commandList->IASetVertexBuffers(0, 1, &vbView);

		///�C���f�b�N�X�o�b�t�@�r���[
		//�C���f�b�N�X�o�b�t�@�r���[�̐ݒ�R�}���h
		commandList->IASetIndexBuffer(&ibView);

		///�萔�o�b�t�@�r���[
		//�萔�o�b�t�@�r���[(CBV)�̐ݒ�R�}���h	//0�Ԗڂ�CBV
		commandList->SetGraphicsRootConstantBufferView(0, constBufferMaterial->GetGPUVirtualAddress());

		//SRV�q�[�v�̐ݒ�R�}���h	//�P�Ԗڂ�SV
		ID3D12DescriptorHeap* ppHeaps[] = {srvHeap.Get()};
		commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		//SRV�q�[�v�̐擪�n���h�����擾(SRV�������Ă���͂�)
		D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle = srvHeap->GetGPUDescriptorHandleForHeapStart();
		
		//1����SRV�q�[�v�̐擪�ɂ���SRV�����[�g�p�����[�^1�Ԃɐݒ�
		if(!IsTexture)
		{	
			srvGpuHandle = srvHeap->GetGPUDescriptorHandleForHeapStart();
			commandList->SetGraphicsRootDescriptorTable(1, srvGpuHandle);
		}
		//2���ڂ��w�������悤�ɂ���SRV�̃n���h�������[�g�p�����[�^1�Ԃɐݒ�
		else if(IsTexture)
		{
			srvGpuHandle.ptr += incrementSize;
			commandList->SetGraphicsRootDescriptorTable(1, srvGpuHandle);
		}

		//�S�I�u�W�F�N�g�ɂ��ď���
		for(size_t i = 0; i < _countof(object3ds); i++)
		{
			DrawObject3d(&object3ds[i], commandList.Get(), vbView, ibView, _countof(indices));
		}


		//4. �`��R�}���h �����܂�


		///���\�[�X�o���A02
		//5. ���\�[�X�o���A��߂�
		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;	//�`���Ԃ���
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;			//�\����Ԃ�
		commandList->ResourceBarrier(1, &barrierDesc);


		///�R�}���h�̃t���b�V��
		//���߃N���[�Y
		result = commandList->Close();
		assert(SUCCEEDED(result));
		//�R�}���h���X�g�̎��s
		ID3D12CommandList* commandLists[] = {commandList.Get()};
		commandQueue->ExecuteCommandLists(1, commandLists);
		
		//��ʂɕ\������o�b�t�@���t���b�v(�\���̓���ւ�)
		result = swapChain->Present(1, 0);
		assert(SUCCEEDED(result));


		///�R�}���h�����҂�
		//�R�}���h�̎��s������҂�
		commandQueue->Signal(fence.Get(), ++fenceVal);
		if(fence->GetCompletedValue() != fenceVal)
		{
			HANDLE event = CreateEvent(nullptr, false, false, nullptr);
			fence->SetEventOnCompletion(fenceVal, event);
			WaitForSingleObject(event, INFINITE);
			CloseHandle(event);
		}

		//�L���[�N���A
		result = commandAllocator->Reset();
		assert(SUCCEEDED(result));
		//�ĂуR�}���h���X�g�𗭂߂鏀��
		result = commandList->Reset(commandAllocator.Get(), nullptr);
		assert(SUCCEEDED(result));


		/// <summary>
		/// DirectX12 ���t���[������ �����܂�
		/// </summary>

	}
	///�E�B���h�E�N���X��o�^����
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
	//�萔�o�b�t�@�̃q�[�v�ݒ�
	D3D12_HEAP_PROPERTIES heapProp{};
	heapProp.Type =D3D12_HEAP_TYPE_UPLOAD;
	//�萔�o�b�t�@�̃��\�[�X�ݒ�
	D3D12_RESOURCE_DESC resDesc{};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = (sizeof(ConstBufferDataTransform)+ 0xff) & ~0xff;
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//�萔�o�b�t�@�̐���
	result = device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&object->constBuffTransform));
	assert(SUCCEEDED(result));

	//�萔�o�b�t�@�̃}�b�s���O
	result = object->constBuffTransform->Map(0,nullptr, (void**)&object->constMapTransform);
	assert(SUCCEEDED(result));
}

void UpdateObject3d(Object3d *object, XMMATRIX &matView, XMMATRIX &matProjection)
{
	XMMATRIX matScale, matRot, matTrans;

	//�X�P�[���A��]�A���s�ړ��s��̌v�Z
	matScale = XMMatrixScaling(object->scale.x, object->scale.y, object->scale.z);
	matRot = XMMatrixIdentity();
	matRot *= XMMatrixRotationZ(object->rotation.z);
	matRot *= XMMatrixRotationX(object->rotation.x);
	matRot *= XMMatrixRotationY(object->rotation.y);
	matTrans = XMMatrixTranslation(object->position.x, object->position.y, object->position.z);

	//���[���h�s��̍���
	object->matWorld = XMMatrixIdentity();	//�ό`�����Z�b�g
	object->matWorld *= matScale;			//���[���h�s��ɃX�P�[�����O�𔽉f
	object->matWorld *= matRot;				//���[���h�s��ɉ�]�𔽉f
	object->matWorld *= matTrans;			//���[���h�s��ɕ��s�ړ��𔽉f

	//�e�I�u�W�F�N�g�̑���
	if(object->parent != nullptr)
	{
		//�e�I�u�W�F�N�g�̃��[���h�s����|����
		object->matWorld *= object->parent->matWorld;
	}

	//�萔�o�b�t�@�ւ̃f�[�^�]��
	object->constMapTransform->mat = object->matWorld * matView *matProjection;
}

void DrawObject3d(Object3d *object, ID3D12GraphicsCommandList* commandList, D3D12_VERTEX_BUFFER_VIEW &vbView, D3D12_INDEX_BUFFER_VIEW &ibView, UINT numIndices)
{
	//���_�o�b�t�@�̐ݒ�
	commandList->IASetVertexBuffers(0, 1, &vbView);
	//�C���f�b�N�X�o�b�t�@�̐ݒ�
	commandList->IASetIndexBuffer(&ibView);
	//�萔�o�b�t�@�r���[(CBV�̐ݒ�R�}���h)
	commandList->SetGraphicsRootConstantBufferView(2, object->constBuffTransform->GetGPUVirtualAddress());
	//�`��R�}���h
	commandList->DrawIndexedInstanced(numIndices,1, 0, 0, 0);
}
