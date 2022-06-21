#pragma once

//#include <Windows.h>

//�L�[�{�[�h����
#define DIRECTINPUT_VERSION 0x0800	//DirectInput�̃o�[�W�����w��
#include <dinput.h>

const int KeyNum = 256;

class Input
{
public:
	Input();
	~Input() = default;

	//������
	void Initialize(HWND hwnd);

	//���t���[��
	void Update();

	//�L�[����check
	bool Push(int keyNumber);
	
	bool Trigger(int keyNumber);

private:
	//�L�[�{�[�h�f�o�C�X
	IDirectInputDevice8* keyboard = nullptr;

	//�L�[���
	BYTE key[KeyNum] = {};
	//�O��̃L�[����
	BYTE prekey[KeyNum] = {};
};

