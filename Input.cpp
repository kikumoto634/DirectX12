#include "Input.h"

#pragma comment(lib, "dinput8.lib")

#include <cassert>

Input::Input()
{
}


void Input::Initialize(HWND hwnd)
{
	HRESULT result;

	///DirectInPut
	//������ (�����͕��@�ǉ��ł����̃I�u�W�F�N�g�͈�̂�)
	IDirectInput8* directInput = nullptr;
	HINSTANCE hInstance = GetModuleHandle(nullptr);
	result = DirectInput8Create(
		hInstance, 
		DIRECTINPUT_VERSION, 
		IID_IDirectInput8,
		(void**)&directInput, 
		nullptr
	);
	assert(SUCCEEDED(result));

	//�L�[�{�[�h�f�o�C�X�̐��� (GUID_Joystick (�W���C�X�e�b�N)�A GUID_SysMouse (�}�E�X))
	//IDirectInputDevice8* keyboard = nullptr;
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
}

void Input::Update()
{
	HRESULT result;

	//����J�n
	result = keyboard->Acquire();
	//assert(SUCCEEDED(result));

	//�O��̃L�[���͏��R�s�[
	for(int i = 0; i <KeyNum; i++){
		prekey[i] = key[i];
	}

	//�L�[����
	result = keyboard->GetDeviceState(sizeof(key), key); 
	//assert(SUCCEEDED(result));
}

bool Input::Push(int keyNumber)
{
	if(keyNumber < 0x00) return false;
	if(keyNumber > 0xff) return false;

	if(key[keyNumber])
	{
		return true;
	}
	return false;
}

bool Input::Trigger(int keyNumber)
{
	if(keyNumber < 0x00) return false;
	if(keyNumber > 0xff) return false;

	if(key[keyNumber] && !prekey[keyNumber])
	{
		return true;
	}
	return false;
}
